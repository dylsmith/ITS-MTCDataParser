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
		all_people[perid].tours[tourid]->numStops >= MinNumStops &&		//Number of stops
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


	for (int i = 0; i < 24; i++)
	{
		//organized[i] = new vector<Trip*>[NUM_LOCATIONS + 1];
		for (int k = 1; k <= NUM_LOCATIONS; k++)
		{
			organized[i][k] = new vector<Trip*>[NUM_LOCATIONS + 1];
			organized[i][k]->reserve(70);
		}
	}

	//origin=destination hour income mode purpose 

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
		}

		all_people[trip.perid].tours[trip.tourid]->trips.push_back(&all_trips[i]);
	}
}

void remove(vector<int>& v, int trip)
{
	vector<int>::iterator it = v.begin();
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
	for (int hour = 0; hour < 24; hour++)
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
								if (++total % 100000000 == 0)
									write("Comparing trips: " + to_string((((double)origin + (NUM_LOCATIONS * hour)) / (NUM_LOCATIONS * 24)) * 100) + "%\n");
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
	cout << "Each trip could share with " << (((long double)sharedtrips / TRIP_FILE_SIZE)) << " other trips, on average." << endl;
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
	//delete[] dist;
	//delete[] all_people;
	//delete[] all_tours;
	//delete[] all_trips;
	//delete[] closePoints;
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

bool canShare(vector<int>* t1Vec, Trip& t2)
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
	return(numPassengers <= MaxPeople && numPassengers >= MinPeople);
}

void shareTrips()
{
	Timer ti("Sharing trips");
	int count = 0;
	//int tenths = TRIP_FILE_SIZE / 100;
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{

		//if ((count++ % tenths) == 0) 
		//	write("Sharing trips " + to_string((double)count / TRIP_FILE_SIZE));

		Trip& t1 = all_trips[t1id];
		if (t1.actualSharing == NULL)
		{
			t1.actualSharing = new vector<int>();
			t1.actualSharing->push_back(t1id);
			for (int t2id : t1.potentialSharing)
			{
				Trip& t2 = all_trips[t2id];
				if (canShare(t1.actualSharing, t2))
				{
					t1.actualSharing->push_back(t2id);
					t2.actualSharing = t1.actualSharing;
				}
			}
		}
	}
}

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

}
void unshare(Trip& t);

void checkTour(Tour& to)
{
	cout << "Checking tour " << to.hhid << endl;
	cout << to.trips.size() << endl;
	if ((to.doableTripCount / to.trips.size()) < TourDoableRequirement)
	{
		for (Trip*& t : to.trips)
		{
			unshare(*t);
		}
	}
}
void checkTours()
{
	Timer ti("Checking Tours");
	int count = 0;
	//int tenths = TOUR_FILE_SIZE / 100;
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		//if (count++ % tenths == 0) write("Checking tours " + to_string((double)count / TOUR_FILE_SIZE));

		Tour& tour = all_tours[i];
		for (Trip*& trip : tour.trips)
		{
			if (trip->actualSharing->size() > 1 || DoableTripModes[trip->mode])
				tour.doableTripCount++;
		}
	}

	
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		Tour& tour = all_tours[i];
		checkTour(tour);
	}
}

void unshare(Trip& t1)
{
	if (t1.actualSharing != NULL)	//If trip has a sharing list (orphaned trips do not)
	{
		int size = t1.actualSharing->size();	//Size of its list (self included)
		if (size > 2) //If remaining trips can still share
		{
			remove(*t1.actualSharing, t1.id);
		}
		else if (size == 2)	//If trip will be orphaned
		{
			remove(*t1.actualSharing, t1.id);
			Trip& t2 = all_trips[t1.actualSharing->at(0)];
			t2.actualSharing = NULL;
		}
		else if (size < 1)
		{
			cout << "Trip with non-null empty actualSharing!" << endl;
		}
	}

}

void averageSharedTrips()
{
	int sharedTrips = 0;
	int unsharedTrips = 0;
	int orphanedTrips = 0;

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.actualSharing)
		{
			if (t.actualSharing->size() > 1)
				sharedTrips++;
			else if (t.actualSharing->size() == 1)
			{
				unsharedTrips++;
			}
		}
		else
		{
			orphanedTrips++;
		}
	}
	cout << sharedTrips << " shared trips" << endl;
	cout << unsharedTrips << " unshared trips" << endl;
	cout << orphanedTrips << " orphaned trips" << endl;
}

void timerWrapper()
{
	Timer total("Total");

	reserveSpace();

	parseClosePoints();

	parsePeople();
	parseTours();
	parseTrips();

	for (int i = 1; i <= PERSON_FILE_SIZE; i++)
	{
		cout << all_people[i].id << endl;
		for (auto to : all_people[i].tours)
		{
			auto* t = to.second;
			cout << "  " << t->id << endl;
			for (Trip* tr : t->trips)
				cout << "    " << tr->id << endl;
		}
	}

	analyzeTrips();

	shareTrips();
	checkTours();

	averageSharedTrips();

}



int _tmain(int argc, _TCHAR* argv[])
{
	timerWrapper();
	
	pause();
	return 0;
}


