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

//Compares two trips, checking every requirements
bool strictCompare(Trip& t1, Trip& t2)
	{
	return (distanceBetween2(t1.origin, t2.origin) < CLOSE_DISTANCE &&
		distanceBetween2(t1.destination, t2.destination) < CLOSE_DISTANCE &&
		t1.perid != t2.perid);
}
		
//Determines if a trip can share with a set of trips
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

//Prototypes for addToSharing
//bool findGroup(Trip& t1); bool formGroup(Trip& t1);
void addToSharing(Trip& t1);

//Checks to see if a tour can be shared again, and does so if needed
void reCheckTour(Tour& to)
	{
	if (!to.shared && to.trips.size() > 0 && (((double)to.doableTripCount / to.trips.size()) >= TourDoableRequirement))
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

//Tries to add a trip back to the sharing groups, recursively following any new additions
void addToSharing(Trip& t1)
{
	bool joinGroup = true;	//Set to true when trying to join a group
	if (t1.actualSharing == NULL && t1.shared)	//If trip is eligible
	{
		for (int t2id : t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (joinGroup && t2.actualSharing) //If t2 has a group
			{
				if (t2.shared && canShare(t2.actualSharing, t1)) //If we can share with t2's group
				{
					t2.actualSharing->push_back(t1.id);
					t1.actualSharing = t2.actualSharing;
					if (t2.actualSharing->size() == 2 && !DoableTripModes[t2.mode])	//If we've just made these trips shareable
					{
						Tour& to = *all_people[t2.perid].tours[t2.tourid];
						to.doableTripCount++;
						reCheckTour(to);
					}
					if (!DoableTripModes[t1.mode])
					{
						Tour& to = *all_people[t1.perid].tours[t1.tourid];
						to.doableTripCount++;
						reCheckTour(to);
					}
					return;
				}
			}
			else
			{
				if (DrivingModes[t1.mode])	//if t1 can be a driver
				{
					if (joinGroup)	//if this is the first time we're in the group-forming stage
					{
						joinGroup = false;
						t1.actualSharing = new list<int>();
						t2.actualSharing = t1.actualSharing;
						t1.actualSharing->push_back(t1.id);
						t1.actualSharing->push_back(t2.id);

						if (!DoableTripModes[t2.mode])
						{
							Tour& to = *all_people[t2.perid].tours[t2.tourid];
							to.doableTripCount++;
							reCheckTour(to);
						}
						if (!DoableTripModes[t1.mode])
						{
							Tour& to = *all_people[t1.perid].tours[t1.tourid];
							to.doableTripCount++;
							reCheckTour(to);
						}
					}
					else //if we've formed a group and are now adding to it
					{
						if (canShare(t1.actualSharing, t2))
						{
							t1.actualSharing->push_back(t2id);
							t2.actualSharing = t1.actualSharing;
						}

						if (!DoableTripModes[t2.mode])
						{
							Tour& to = *all_people[t2.perid].tours[t2.tourid];
							to.doableTripCount++;
							reCheckTour(to);
						}
					}
				}
			}
		}
	}
	if (t1.actualSharing == NULL)	//If we failed to find or form a group, make a solo trip
	{
		t1.actualSharing = new list<int>();
		t1.actualSharing->push_back(t1.id);
	}
}

//Tries to form a new group around a trip
bool formGroup(Trip& t1)	//Returns true if at least one other trip is added. Will always create t1.actualSharing of some size >= 1
{
	if (DrivingModes[t1.mode])//If t1 can drive
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
				if (t1.actualSharing->size() == 2 && !DoableTripModes[t1.mode])//If we've just made these trips shareable
				{
					Tour& to = *all_people[t1.perid].tours[t1.tourid];
					to.doableTripCount++;
					reCheckTour(to);
				}
				if (!DoableTripModes[t2.mode])
				{
					Tour& to = *all_people[t2.perid].tours[t2.tourid];
					to.doableTripCount++;
					reCheckTour(to);
				}
			}
		}
	}
	return t1.actualSharing != NULL;
}

//Prototypes for removeFromSharing
void checkTour(Tour& to);

//Tries to remove a trip from the sharing groups, recursively following any removals
//Check to make sure there's still a driver
void removeFromSharing(Trip& t1)
{
	if (t1.shared)	//if t1 hasn't already been unshared
	{
		t1.shared = 0;
		if (!DoableTripModes[t1.mode])
		{
			Tour& to1 = *all_people[t1.perid].tours[t1.tourid];
			to1.doableTripCount--;
			checkTour(to1);
		}

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

				remove(*t1.actualSharing, t2id);
				Trip& t2 = all_trips[t2id];
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
				if (DrivingModes[t1.mode])
				{
					bool foundNewDriver = false;
					for (int t2id : *t1.actualSharing)
					{
						Trip& t2 = all_trips[t2id];
						if (DrivingModes[t2.mode] && t2id != t1.id)
						{
							foundNewDriver = true;
							break;
						}
					}
					if (!foundNewDriver)
					{
						for (int t2id : *t1.actualSharing)
							removeFromSharing(all_trips[t2id]);
					}
				}

				
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

//Checks to see if a tour cannot be shared, and unshares it if needed
void checkTour(Tour& to)
{
	if (to.shared && to.trips.size() > 0 && (((double)to.doableTripCount / to.trips.size()) < TourDoableRequirement))
	{
		to.shared = 0;
		for (Trip*& t : to.trips)
		{
			removeFromSharing(*t);
		}
	}
}

//Tries to form actual sharing groups for drivers
void shareTrips()
{
	Timer ti("Sharing trips");
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.actualSharing == NULL)
		{
			formGroup(t1);//figure out how to associate formGroup and drivers. probably needs to be split, because this requires drivers to have groups made and not non-drivers
		}
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

		Tour& tour = all_tours[i];
		for (Trip*& trip : tour.trips)
		{
			if (DoableTripModes[trip->mode])
				tour.doableTripCount++;
		}
	}

	
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		checkTour(all_tours[i]);
	}

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
		if (all_trips[i].actualSharing && all_trips[i].actualSharing->size() > 1) sharingBeforeReshare++;
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

	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.leader == NULL && DrivingModes[t1.mode])
		{
			t1.leader = &t1;
			for (int t2id : *t1.actualSharing)
			{
				all_trips[t2id].leader = &t1;
			}
			actualSharing++;
		}
	}
}

//Gathers data after algorithms are finished
void postStatistics()
{
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

void tripDetailsOutput()
{
	Timer ti("Writing trip details");

	ifstream inf(TRIP_FILE);
	string* lines;
	lines = new string[TRIP_FILE_SIZE];
	string line;
	inf >> line;
	int count = 0;
	while (getline(inf, line))
	{
		lines[count++] = line;
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
			if (t.leader)
			{
				sharedCount += t.actualSharing->size();
				shared << lines[i] << endl;
			}
			else
			{
				unsharedCount++;
				unshared << lines[i] << endl;
			}
		}
	}
	/*
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.actualSharing->size() > 1)
		{
			if (t.leader == &t)
			{
				sharedCount += t.actualSharing->size();
				shared << lines[i];
			}
		}
		else
		{
			unsharedCount++;
			unshared << lines[i];
		}
	}	*/


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
	checkTours();	
	shareTrips2();
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

