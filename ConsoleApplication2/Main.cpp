#include "stdafx.h"


#include "Globals.h"
#include "DataClasses.h"
#include "FastRand.h"
#include "MiscFunctions.h"
#include "QuickParser.h"
#include "QuickParser_inline.cpp"
#include "Timer.h"
#include "LoadData.h"

#include <iostream>
#include "omp.h"
using namespace std;

//Returns true if trip passes initial parameter checks
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

//Compares trips, assuming they're potentially shareable already
inline bool compareTrips(Trip& trip1, Trip& trip2)
{
	return (trip1.perid != trip2.perid);
}

//Parses the sorted trips, builds potential sharing lists
void analyzeTrips()
	{
	Timer ct("Analyzing trips");

	long long int sharedtrips = 0;
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

//Reserves space for all data
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

//Frees space
void cleanUp()
{
	free((void*)close);
	for (int i = 0; i < 24; i++)
		for (int k = 1; k <= NUM_LOCATIONS; k++)
			delete organized[i][k];
}

//Prototypes for addToSharing
//bool findGroup(Trip& t1); bool formGroup(Trip& t1);
void addToSharing(Trip& t1);

//Checks to see if a tour can be shared again, and does so if needed
void reCheckTour(Tour& to)
{
	int doableTripCount = 0;
	for (Trip* t : to.trips)
		if (t->doable)
			doableTripCount++;

	if (!to.shared && to.trips.size() > 0 && (((double)doableTripCount / to.trips.size()) >= TourDoableRequirement))
	{
		to.shared = 1;
		for (Trip*& t : to.trips)
		{
			if (!t->shared && t->isShareable())
			{
				t->shared = 1;
				addToSharing(*t);
			}
		}
	}
}

void Trip::setDoable(bool set)
{
	if (set == true && doable == false) //if we're making the trip doable
	{
		doable = true;
		Tour& to = *all_people[perid].tours[tourid];
		reCheckTour(to);
	}
	else if (set == false && doable == true)//if we're makinga trip not doable
	{
		if (!DoableTripModes[mode])
		{
			doable = false;
			Tour& to = *all_people[perid].tours[tourid];
			reCheckTour(to);
		}
	}
}

//Tries to add a trip back to the sharing groups, recursively following any new additions

void addToSharing(Trip& t1)
{
	if (t1.actualSharing == NULL && t1.shared)
	{
		for (int t2id : t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (t2.shared && t2.actualSharing && canShare(t2.actualSharing, t1))
			{
				t2.actualSharing->push_back(t1.id);
				t1.actualSharing = t2.actualSharing;
				t1.setDoable(true);
				t2.setDoable(true);
				return;
			}
		}

		t1.actualSharing = new list<int>;
		t1.actualSharing->push_back(t1.id);
		if (DrivingModes[t1.mode])
		{
			for (int t2id : t1.potentialSharing)
			{
				Trip& t2 = all_trips[t2id];
				if (!t2.actualSharing && t2.shared && canShare(t1.actualSharing, t2))
				{
					t1.actualSharing->push_back(t2id);
					t2.actualSharing = t1.actualSharing;
					t1.setDoable(true);
					t2.setDoable(true);
				}
			}
		}
	}
}

//Tries to form a new group around a trip
bool formGroup(Trip& t1)	//Returns true if at least one other trip is added. Will always create t1.actualSharing of some size >= 1
{
	if (DrivingModes[t1.mode] && t1.actualSharing == NULL)//If t1 can drive
	{
		t1.actualSharing = new list<int>();
		t1.actualSharing->push_back(t1.id);
		for (int t2id : t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (t2.actualSharing == NULL && t2.shared && canShare(t1.actualSharing, t2))//if t2 is eligible and can share
			{
				t1.actualSharing->push_back(t2id);
				t2.actualSharing = t1.actualSharing;
				t1.setDoable(true);
				t2.setDoable(true);
			}
		}
	}
	return t1.actualSharing != NULL;
}

//Prototypes for removeFromSharing
void checkTour(Tour& to);


//Tries to remove a trip from the sharing groups, recursively following any removals
/*
void removeFromSharing(Trip& t1)
{
	if (t1.shared)	//if t1 hasn't already been unshared
	{
		t1.shared = 0;
		t1.setDoable(false);

		if (t1.potentialSharing.size() > 0)
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
				Trip& t2 = all_trips[t2id];

				remove(*t1.actualSharing, t2id);

				t2.actualSharing = NULL;
				t2.setDoable(false);
			}
			else if (size > 2) //size > 2
			{
				remove(*t1.actualSharing, t1.id);
				t1.actualSharing = new list<int>();
				t1.actualSharing->push_back(t1.id);
				
				if (DrivingModes[t1.mode])
				{
					auto& actualSharing = *t1.actualSharing;
					bool foundNewDriver = false;
					for (int t2id : actualSharing)
					{
						if (DrivingModes[all_trips[t2id].mode])
						{
							foundNewDriver = true;
							break;
						}
					}
					if (!foundNewDriver)
					{
						list<int>& sharing = actualSharing;
						list<int>::iterator i = sharing.begin();
						while (i != sharing.end())
						{
							int curSize = t1.actualSharing->size();
							removeFromSharing(all_trips[*i++]);
							if (curSize != t1.actualSharing->size());
								i = t1.actualSharing->begin();
							
						}
					}
				}		

			}
		}
		else
		{
			t1.actualSharing = new list<int>();
			t1.actualSharing->push_back(t1.id);
		}
	}
}*/

//Checks to see if a tour cannot be shared, and unshares it if needed
void checkTour(Tour& to)
{
	int doableTripCount = 0;
	for (Trip* t : to.trips)
		if (t->doable)
			doableTripCount++;

	if (to.shared && to.trips.size() > 0 && (((double)doableTripCount / to.trips.size()) < TourDoableRequirement))
	{
		to.shared = 0;
		for (Trip*& t : to.trips)
		{
			t.group.removeTrip(*t);
			//removeFromSharing(*t);
		}
	}
}

//Tries to form actual sharing groups for drivers
void shareTrips()
{
	Timer ti("Sharing trips");
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		formGroup(all_trips[t1id]);
	}

	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.actualSharing == NULL)
		{
			t1.actualSharing = new list<int>();
			t1.actualSharing->push_back(t1id);
		}
		else if (t1.actualSharing->size() > 1)
		{
			sharingBeforeTourLevel++;
		}
	}
}

//Checks to ensure all tours can be shared
void checkTours()
{
	Timer ti("Checking Tours");

	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		checkTour(all_tours[i]);
	}

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
		if (all_trips[i].actualSharing && all_trips[i].actualSharing->size() > 1) sharingBeforeReshare++;
}
	
void generateLeaders(bool clearAfter = false)
{
	int actualSharing2 = 0;
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.actualSharing && t1.leader == NULL && DrivingModes[t1.mode])
		{
			for (int t2id : *t1.actualSharing)
			{
				all_trips[t2id].leader = &t1;
			}
			if (t1.actualSharing->size() > 1)
				groups++;
		}
	}

	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.actualSharing)
		{
			if (t1.actualSharing->size() > 1)
			{
				actualSharing++;
				if (t1.leader == &t1)
				{
					actualSharing2 += t1.actualSharing->size();
				}
			}
			if (find(t1.actualSharing->begin(), t1.actualSharing->end(), t1.id) == t1.actualSharing->end())
				cout << t1id << " was not in its actualsharing set." << endl;
		}
	}
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		remove(*t1.actualSharing, t1.id);
	}
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.actualSharing->size() > 0)
			cout << t1id << " had extra elements in its sharing list" << endl;
	}
	cout << "1: " << actualSharing << "   2: " << actualSharing2 << endl;

	if (clearAfter)
	{
		for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
			all_trips[t1id].leader = NULL;
		actualSharing = 0;
		groups = 0;
	}
}

//Attempts to re-share trips after tour-level removals
void shareTrips2()
{
	Timer ti("Resharing trips");
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.isShareable() && t1.actualSharing == NULL)
		{
			addToSharing(t1);
		}
	}


}

//Gathers data after algorithms are finished
void postStatistics()
{
	Timer ti("Writing output.txt");
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.potentialSharing.size() > 0)
		{
			potentialSharing++;
		}

		if (t.actualSharing)
		{
			if (t.actualSharing->size() > 1)
			{
				//actualSharing++;
				for (int k : *t.actualSharing)
				{
					Trip& t2 = all_trips[k];
					if (DrivingModes[t2.mode] && t2.leader != &t2)
					{
						VMTReduction += distanceBetween2(t2.origin, t2.destination);
					}
				}
			}
			else
			{
				solo++;
			}


		}
		else
		{
			if (t.isShareable())
				orphaned++;
		}
	}

	ofstream outf(DATA_FILE);
	if (Maximize)
		outf << "Maximizing group size" << endl;
	else
		outf << "Minimizing group size" << endl;

	outf << "Total trips: " << TRIP_FILE_SIZE << endl;
	outf << "Shareable trips: " << shareable << endl;
	outf << "Trips with at least one potentially shared trip: " << potentialSharing << endl;
	outf << "Trips with at least one actually shared trip (before tour-level checks): " << sharingBeforeTourLevel << endl;
	outf << "Trips with at least one actually shared trip (before re-sharing): " << sharingBeforeReshare << endl;
	outf << "Trips with at least one actually shared trip: " << actualSharing << endl;
	outf << "Trips with no actually shared trips: " << solo << endl;
	outf << "Total sharing groups: " << groups << endl;
	outf << "Trips unshared at the tour-level: " << unshared << endl;
	outf << "Orphaned trips (should be 0): " << orphaned << endl;
	outf << "Vehicle miles reduction: " << VMTReduction << endl;
	outf.close();
}

//Writes output related to each trip's sharing to a file
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
				outf << ',' << t2id;
		}
	}
	outf << '\n';
}

string lineModify(string input, string numPassengers, string mode)
{
	int commas = 0;
	int loc = -1;
	int start, end;

	while (commas != 2)
		if (input[++loc] == ',')
			commas++;
	start = loc;
	while (commas != 3)
		if (input[++loc] == ',')
			commas++;
	end = loc;

	input = input.substr(0, start + 1) + numPassengers + input.substr(end, input.size() - end);

	commas = 0;
	loc = -1;
	while (commas != 15)
		if (input[++loc] == ',') 
			commas++;
	start = loc;
	while (commas != 16)
		if (input[++loc] == ',') 
			commas++;
	end = loc;

	return input.substr(0, start + 1) + mode + input.substr(end, input.size() - end);
}

void tripDetailsOutput()
{
	Timer ti("Writing trip details");

	ifstream inf(TRIP_FILE);
	string* lines;
	lines = new string[TRIP_FILE_SIZE];
	string line;
	getline(inf, line);
	int count = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		getline(inf, line);
		lines[count] = line;
		++count;
	}

	ofstream shared(SHARED_TRIPS_FILE);
	ofstream unshared(UNSHARED_TRIPS_FILE);
	
	int sharedCount = 0;
	int unsharedCount = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.actualSharing)
		{
			if (t.actualSharing->size() > 1)
			{
				//sharedCount++;
				if (t.leader == &t)
				{
					sharedCount += t.actualSharing->size();// += t.actualSharing->size();
					int numPassengers = 0;
					for (int i : *t.actualSharing)
					{
						Trip& t2 = all_trips[i];
						numPassengers += t2.numPassengers;
					}
					shared << lineModify(lines[i], to_string(numPassengers), "5") << endl;
				}
			}
			else
			{
				unsharedCount++;
				//unshared << lines[i] << endl;
			}
		}
	}

	cout << "shared: " << sharedCount << endl << "unshared: " << unsharedCount << endl;
}

//Records total execution time
void timerWrapper()
{
	Timer total("Total");

	reserveSpace();

	parseClosePoints();

	parsePeople();
	parseTours();
	parseTrips();
	analyzeTrips();


	shareTrips();
	generateLeaders(true);
	checkTours();	
	//shareTrips2();
	generateLeaders();
	postStatistics();

	tripDetailsOutput();

	//tripSharingOutput(); //Output a list of each trip and the trips it's actually shared with

	//output a .csv with all shared drivers' trip info and one with unsharedtrips
	//output a list of all shared people
	//output a list of all trips




}

//output: trip (split by sharing and not sharing)

//Main
int _tmain(int argc, _TCHAR* argv[])
{
	
	timerWrapper();
	
	cout << "Finished." << endl;
	pause();
	return 0;
}


/*
fix VMT miles
fix shared trip output
unify shared & unshared trip otuput
generate trips + actual sharing sets file
print each person and their probability data

*/

