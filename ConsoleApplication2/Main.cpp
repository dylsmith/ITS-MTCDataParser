#include "stdafx.h"


#include "Globals.h"
#include "DataClasses.h"
#include "FastRand.h"
#include "MiscFunctions.h"
#include "QuickParser.h"
#include "QuickParser_inline.cpp"
#include "Timer.h"

#include <iostream>
#include "omp.h"
using namespace std;

double distanceBetween2(int origin, int destination)
{
	return dist[((origin - 1) * NUM_LOCATIONS) + (destination - 1)];
}

bool Trip::isShareable()
{

	if (shareable == UNKNOWN)
		shareable = (
		all_people[perid].tours[tourid]->numStops <= MaxNumStops &&		//Number of stops
		distanceBetween2(origin, destination) > MinDistanceTraveled &&	//Trip length
		all_people[perid].income < MaxIncome &&							//Income level
		TripModes[mode] == 1 &&											//Mode is valid check  (array index of the mode must be 1)
		((fastrand() % 100) + 1) > RandomFailChance &&					//Random chance (random int must be greater than fail chance)
		TripPurposes.find(purpose) != TripPurposes.end()				//Trip purpose (purpose must be in the set of allowed purposes)
		);
	return shareable;

}

void parseClosePoints()
{

	Timer timeit("Parsing ClosePoints");

	QuickParser q(DISTANCE_FILE);
	int j = 0;
	while (j < DISTANCE_FILE_SIZE)
	{
		q.parseNewLine();
		q.parseComma();
		q.parseComma();
		dist[j++] = q.parseFloat();
	}

	//Check non-diagonal points where k > i. Graph is symmetric, so we don't need to check k < i
	for (int i = 1; i <= NUM_LOCATIONS; i++)
	{
		closePoints[i].push_back(i);
		for (int k = i + 1; k <= NUM_LOCATIONS; k++)
		{

			if (distanceBetween2(i, k) < CLOSE_DISTANCE)
			{
				closePoints[i].push_back(k);
				closePoints[k].push_back(i);
				close[i][k] = 1;
				close[k][i] = 1;
			}
		}
	}
}
void parsePeople()
{
	QuickParser q(PERSON_FILE);

	Timer timeit("Parsing people");

	for (int i = 0; i < PERSON_FILE_SIZE; i++)
	{
		q.parseNewLine();

		q.parseComma();
		int perid = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		all_people[perid].income = q.parseInt();

		all_people[perid].id = perid;
	}
}
void parseTours()
{
	QuickParser q(TOUR_FILE);

	Timer timeit("Parsing tours");

	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		q.parseNewLine();
		
		int hhid = q.parseInt();
		int perid = q.parseInt();
		q.parseComma();
		q.parseComma();
		int tourid = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		all_tours[i].numStops = q.parseInt() + q.parseInt(); //Outbound stops + inbound stops


		all_tours[i].hhid = hhid;
		all_tours[i].id = tourid;
		all_people[perid].tours[tourid] = &all_tours[i];
	}
}
void parseTrips()
{
	QuickParser q(TRIP_FILE);
	Timer timeit("Parsing trips");

	for (int i = 5; i < 24; i++)
	{
		for (int k = 1; k <= NUM_LOCATIONS; k++)
		{
			organized[i][k] = new vector<Trip*>[NUM_LOCATIONS + 1];
			organized[i][k]->reserve(70);
		}
	}

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& trip = all_trips[i];

		q.parseNewLine();

		q.parseComma();
		trip.perid = q.parseInt();
		trip.numPassengers = q.parseInt();
		trip.tourid = q.parseInt();
		q.parseComma();
		q.parseComma();
		trip.purpose = q.parseString();
		q.parseComma();
		q.parseComma();
		trip.origin = q.parseInt();
		q.parseComma();
		trip.destination = q.parseInt();
		q.parseComma();
		q.parseComma();
		trip.hour = q.parseInt();
		trip.mode = q.parseInt();
		all_trips[i].id = i;

		if (trip.isShareable())
		{
			organized[trip.hour][trip.origin][trip.destination].push_back(&all_trips[i]);
			shareable++;
		}
		else
		{
			trip.shared = 0;
		}

		all_people[trip.perid].tours[trip.tourid]->trips.push_back(&all_trips[i]);
	}
}


//Generic remove function for STL classes that dont' support find() (linear search)
template<class T>
void remove(T& v, int trip)
{
	T::iterator it = v.begin();
	while (*it != trip)
		it++;
	v.erase(it);
}

inline bool compareTrips(Trip& trip1, Trip& trip2)
{
	return (trip1.perid != trip2.perid);
}

void analyzeTrips()
{
	Timer ct("Comparing trips");

	long long int sharedtrips = 0;
	long long int total = 0;
	for (int hour = 5; hour < 24; hour++)
	{
		for (int origin = 1; origin <= 12; origin++)
		{
			for (int destination = 1; destination <= 12; destination++)
			{
				for (Trip* trip1 : organized[hour][origin][destination])
				{
					for (int closeOrigin : closePoints[origin])
					{
						for (int closeDestination : closePoints[destination])
						{
							for (Trip* trip2 : organized[hour][closeOrigin][closeDestination])
							{
								//if (++total % 100000000 == 0)
								//	write("Comparing trips: " + to_string((((double)origin + (NUM_LOCATIONS * hour) - (NUM_LOCATIONS * 5)) / (NUM_LOCATIONS * 19)) * 100) + "%\n");
								//if (++total % 100000000 == 0) cout << "Comparing trips: " << (((double)origin + (NUM_LOCATIONS * hour)) / (NUM_LOCATIONS * 24)) * 100 << "%" << endl;
								if (compareTrips(*trip1, *trip2))
								{
									trip1->potentialSharing.push_back(trip2->id);
									sharedtrips++;
								}
							}
						}
					}
				}
			}
		}
	}
	cout << "Each trip could potentially share with " << (((long double)sharedtrips / TRIP_FILE_SIZE)) << " other trips, on average." << endl;
}

void reserveSpace()
{
	Timer t("Reserving space for all objects");

	memset(close, 0, DISTANCE_FILE_SIZE);
	dist = new float[DISTANCE_FILE_SIZE];
	all_people = new Person[PERSON_FILE_SIZE + 1];
	all_tours = new Tour[TOUR_FILE_SIZE];
	all_trips = new Trip[TRIP_FILE_SIZE];
	closePoints = new vector<short>[NUM_LOCATIONS + 1];
}

void cleanUp()
{
	free((void*)close);
	for (int i = 0; i < 24; i++)
		for (int k = 1; k <= NUM_LOCATIONS; k++)
			delete organized[i][k];
}

bool strictCompare(Trip& t1, Trip& t2)
{
	return (distanceBetween2(t1.origin, t2.origin) < CLOSE_DISTANCE &&
		distanceBetween2(t1.destination, t2.destination) < CLOSE_DISTANCE &&
		t1.perid != t2.perid);
	//Similar origin and destination, different perid, etc.
}

bool canShare(list<int>* t1Vec, Trip& t2)
{
	int numPassengers = 0;
	for (int t1id : *t1Vec)
	{
		Trip& t1 = all_trips[t1id];
		if (!strictCompare(t1, t2))
		{
			return false;
		}
		numPassengers += t1.numPassengers;
	}
	numPassengers += t2.numPassengers;
	
	if (Maximize)
		return(numPassengers <= MaxPeople);	//Move this check to after trip set generation
	else
		return(numPassengers >= MinPeople);
}

void shareTrips()
{
	Timer ti("Sharing trips");
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.shared && t1.actualSharing == NULL && DrivingModes[t1.mode])
		{
			t1.actualSharing = new list<int>();
			t1.actualSharing->push_back(t1id);
			for (int t2id : t1.potentialSharing)
			{
				Trip& t2 = all_trips[t2id];
				if (t2.actualSharing == NULL && canShare(t1.actualSharing, t2))
				{
					t1.actualSharing->push_back(t2id);
					t2.actualSharing = t1.actualSharing;
				}
			}
		}
	}
}

/*
void shareTrips2()
{
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.actualSharing == NULL)
		{
			for (int t2id : t1.potentialSharing)
			{
				Trip& t2 = all_trips[t2id];
				if (t2.actualSharing != NULL)
				{
					if (canShare(t2.actualSharing, t1))
					{
						t1.actualSharing = t2.actualSharing;
						t1.actualSharing->push_back(t1id);
						return;
					}
				}
				else
				{
					t1.actualSharing = new vector<int>();
					t2.actualSharing = t1.actualSharing;
					t1.actualSharing->push_back(t1id);
					t1.actualSharing->push_back(t2id);
					return;
				}
			}
			t1.actualSharing = new vector<int>();
			t1.actualSharing->push_back(t1id);
		}
	}

}*/

void unshare(Trip& t);

void checkTour(Tour& to)
{
	if (to.shared && to.trips.size() > 0 && (((double)to.doableTripCount / to.trips.size()) < TourDoableRequirement))
		{
			to.shared = 0;
			for (Trip*& t : to.trips)
			{
				unshare(*t);
			}
		}
	}

void checkTours()
{
	Timer ti("Checking Tours");
	int hundr = TOUR_FILE_SIZE / 100;
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{

		Tour& tour = all_tours[i];
		for (Trip*& trip : tour.trips)
		{
			if ((trip->actualSharing != NULL && trip->actualSharing->size() > 1) || DoableTripModes[trip->mode])
				tour.doableTripCount++;
		}
	}

	
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		if (i % hundr == 0)
			cout << "Checking tours: " << round((double)i / TOUR_FILE_SIZE * 100) << "%" << endl;
		checkTour(all_tours[i]);
	}
}
/*
void unshare(Trip& t1)
{
	if (t1.actualSharing != NULL)
	{
		int size = t1.actualSharing->size();
		if (size > 2)
		{
			remove(*t1.actualSharing, t1.id);
			t1.actualSharing = NULL;
		}
		else if (size == 2)
		{
			remove(*t1.actualSharing, t1.id);
			Trip& t2 = all_trips[t1.actualSharing->at(0)];
			delete t1.actualSharing;
			t2.actualSharing = NULL;
			t1.actualSharing = NULL;
			for (int t3id : t2.potentialSharing)
			{
				Trip& t3 = all_trips[t3id];
				if (t3.actualSharing != NULL && canShare(t3.actualSharing, t2))
				{
					t3.actualSharing->push_back(t2.id);
					t2.actualSharing = t3.actualSharing;
				}
			}
			if (t2.actualSharing == NULL)
			{
				t2.actualSharing = new vector<int>();
				t2.actualSharing->push_back(t2.id);
				if (!DoableTripModes[t2.mode])
				{
					Tour& to2 = *all_people[t2.perid].tours[t2.tourid];
					to2.doableTripCount--;
					checkTour(to2);
				}
			}
		}
		else //size = 1 or 0
		{
			delete t1.actualSharing;
		}
		t1.actualSharing = NULL;
	}
}
}*/


/*/
int totalUnshared = 0;
void unshare(Trip& t1)
{
	if (t1.actualSharing != NULL)	//If trip has a sharing list (orphaned trips do not)
	{
		totalUnshared++;
		int size = t1.actualSharing->size();	//Size of its list (self included)
		if (size > 2) //If remaining trips can still share
		{
			remove(*t1.actualSharing, t1.id);

		}
		else if (size == 2)	//If trip will be orphaned
		{
			remove(*t1.actualSharing, t1.id);
			Trip& t2 = all_trips[t1.actualSharing->at(0)];
			if (!DoableTripModes[t2.mode])
			{
				Tour& to2 = *all_people[t2.perid].tours[t2.tourid];
				to2.doableTripCount--;
				checkTour(to2);
			}
		
			t2.actualSharing = NULL;
			delete t1.actualSharing;
		}
		else if (size < 1)
		{
			cout << "Trip with non-null empty actualSharing!" << endl;
		}
	}
	t1.actualSharing = new vector<int>();
	t1.actualSharing->push_back(t1.id);	//Make sure this trip is not re-shared

}*/
list<int> orphanedTrips;
long int unshareCount;
void unshare(Trip& t1)
{
	if (t1.shared)
	{
		t1.shared = 0;
		unshared++;
		if (t1.actualSharing)
		{
			int size = t1.actualSharing->size();

			if (size == 2)
			{
				int t2id;
				if (*t1.actualSharing->begin() == t1.id)
					t2id = *t1.actualSharing->rbegin();
				else
					t2id = *t1.actualSharing->begin();

				remove(*t1.actualSharing, t2id);
				Trip& t2 = all_trips[t2id];
				orphanedTrips.push_back(t2id);
				t2.actualSharing = NULL;
				if (!DoableTripModes[t2.mode])
				{
					Tour& to2 = *all_people[t2.perid].tours[t2.tourid];
					to2.doableTripCount--;
					checkTour(to2);
				}
			}
			else if (size > 2) //size > 2
			{
				remove(*t1.actualSharing, t1.id);
				t1.actualSharing = new list<int>();
				t1.actualSharing->push_back(t1.id);
			}
		}
		else
		{
			t1.actualSharing = new list<int>();
			t1.actualSharing->push_back(t1.id);
		}
	}
}


void reshare()
{
	Timer ti("Resharing trips");
	for (int t1id : orphanedTrips)
	{
		
		Trip& t1 = all_trips[t1id];
		t1.actualSharing = NULL;
		for (int t2id : t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (t2.shared && t2.actualSharing != NULL && canShare(t2.actualSharing, t1))
			{
				t2.actualSharing->push_back(t1.id);
				t1.actualSharing = t2.actualSharing;
			}
		}
		if (t1.actualSharing == NULL)
		{
			t1.actualSharing = new list<int>();
			t1.actualSharing->push_back(t1.id);
		}
	}
	orphanedTrips.clear();
}

/*
	shared = false
		actualSharing == NULL
			not initially shareable
		actualSharing exists
			unshared at tour level
	shared = true
		actualSharing = NULL
			orphaned at tour level
		actualSharing size = 1
			could not be actually shared
		actualSharing size > 1
			was actually shared
*/
void postStatistics()
{
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.shared)
		{
			if (t.potentialSharing.size() > 0)
				potentialSharing++;

			if (t.actualSharing)
			{
				if (t.actualSharing->size() == 1)
					solo++;
				else if (t.actualSharing->size() > 1)
				{
					actualSharing++;
					for (int k : *t.actualSharing)
					{
						Trip& t2 = all_trips[k];
						if (DrivingModes[t2.mode])
						{
							VMTReduction += distanceBetween2(t2.origin, t2.destination);
						}
					}
				}
			}
			else
			{
				orphaned++;
			}
		}
		else
		{
			if (t.actualSharing)
			{
				unshared++;
			}
			else
			{
				//was not initially shareable
			}
		}
		/*
		if (t.isShareable())
		{
			if (!t.actualSharing)
			{
				orphaned++;
			}
			else
			{
				if (t.actualSharing->size() > 1)
				{
					actualSharing++;
					for (int k : *t.actualSharing)
					{
						Trip& t2 = all_trips[k];
						if (DrivingModes[t2.mode])
						{
							VMTReduction += distanceBetween2(t2.origin, t2.destination);
						}
					}
				}
				else if (t.actualSharing->size() == 1)
				{
					solo++;
				}
			}
			if (t.potentialSharing.size() > 1)
			{
				potentialSharing++;
			}
		}*/
	}

	ofstream outf(DATA_FILE);
	if (Maximize)
		outf << "Maximizing group size" << endl;
	else
		outf << "Minimizing group size" << endl;

	outf << "Total trips: " << TRIP_FILE_SIZE << endl;
	outf << "Shareable: " << shareable << endl;
	outf << "potentialSharing: " << potentialSharing << endl;
	outf << "actualSharing: " << actualSharing << endl;
	outf << "unshared: " << unshared << endl;
	outf << "solo: " << solo << endl;
	outf << "orphaned: " << orphaned << endl;
	outf << "Vehicle miles reduction: " << VMTReduction << endl;

	/*
	outf << "Total trips: " << TRIP_FILE_SIZE << endl;
	outf << "Shareable trips: " << shareable << endl;
	outf << "Trips that can potentially share with at least one other: " << potentialSharing << endl;
	outf << "Trips that actually shared with at least one other: " << actualSharing << endl;
	outf << "Trips unshared at tour-level: " << unshared << endl;
	outf << "Trips that were not shared, but could have been: " << solo << endl;
	outf << "Orphaned trips: " << orphaned << endl;
	outf << "Vehicle miles reduction: " << VMTReduction << endl;
	*/
	outf.close();
}

void tripSharingOutput()
{
	Timer ti("Writing trip sharing to file");
	ofstream outf(TRIP_SHARING_FILE);
	for(int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		outf << t1id;
		if (all_trips[t1id].actualSharing)
		{
			for (int t2id : *all_trips[t1id].actualSharing)
			{
				outf << ',' << t2id;
			}
		}
		outf << '\n';
	}
}

void timerWrapper()
{
	Timer total("Total");

	reserveSpace();

	parseClosePoints();

	parsePeople();
	parseTours();
	parseTrips();	
	analyzeTrips();	
	

	shareTrips();	//Share all possible trips, split based on max and min later
	checkTours();	//Loop until 5% or lass are removed (ensure not to add to unshareed trips)
	//reshare();
	postStatistics();

	tripSharingOutput(); //Output a list of each trip and the trips it's actually shared with

	//output a .csv with all shared drivers' trip info and one with unsharedtrips
	//output a list of all shared people
	//output a list of all trips


	/*
	log file:
		Max or Min
		# of trips
		# valid trpis
		# of trips with nonzero potential sharing
		# o frips with nonzero actual sharing
		# of trips removed at tour level
		# of trips orphaned at tour level
		# of orphaned trips re-added //this will loop until 5%
		//VMT reduction 

	*/


}

//output: trip (split by sharing and not sharing)

int _tmain(int argc, _TCHAR* argv[])
{
	timerWrapper();
	
	cout << "Finished." << endl;
	pause();
	return 0;
}


