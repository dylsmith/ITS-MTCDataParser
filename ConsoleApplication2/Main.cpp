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

//Prototypes for addToSharing
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

//Tries to add a trip back to the sharing groups
void addToSharing(Trip& t1)
{

	//if (t1.shared && (!t1.group || (t1.group && t1.group->trips.size() == 1)))
	if (t1.group == NULL)
	{
		for (int t2id : t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (t2.group && t2.group->canAddTrip(t1))
			{
				t2.group->addTrip(t1, false);
				return;
			}
		}

		if (t1.group == NULL)
			t1.group = new VGroup(t1);

		if (DrivingModes[t1.mode])
		{
			for (int t2id : t1.potentialSharing)
			{
				Trip& t2 = all_trips[t2id];
				if (t1.group->canAddTrip(t2))
				{
					t1.group->addTrip(t2, false);
				}
			}
		}
	}
}

//Tries to form a new group around a trip
void formGroup(Trip& t1)	//Returns true if at least one other trip is added. Will always create t1.actualSharing of some size >= 1
{
	if (DrivingModes[t1.mode] && t1.group == NULL)
	{
		t1.group = new VGroup(t1);
		for (int t2id : t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (t1.group->canAddTrip(t2))
			{
				t1.group->addTrip(t2, false);
			}
		}
	}
}

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
		vector<Trip*>::iterator it = to.trips.begin();
		while (it != to.trips.end())
		{
			VGroup* group = (*it)->group;

			if (group != NULL)
			{
				int size = group->trips.size();
				group->removeTrip(**it);
				if (size != (*it)->group->trips.size())
					it = to.trips.begin();
				else
					it++;
			}
			else
				it++;
		}
		/*
		for (Trip*& t : to.trips)
		{
			if (t->group != NULL)
				t->group->removeTrip(*t);
		}*/
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
		if (t1.group == NULL)
		{
			t1.group = new VGroup(t1);
		}
		else if (t1.group->trips.size() > 1)
		{
			sharingBeforeTourLevel++;
		}
	}
}

//Checks to ensure all tours can be shared
void checkTours()
{
	Timer* ti = new Timer("Checking Tours");

	int prevUnshared = unshared;
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		checkTour(all_tours[i]);
	}

	if (prevUnshared != unshared)
	{
		delete ti;
		cout << unshared - prevUnshared << " unshared. Going again: " << endl;
		checkTours();
	}
	else
	{
		cout << unshared - prevUnshared << " unshared." << endl;
		for (int i = 0; i < TRIP_FILE_SIZE; i++)
			if (all_trips[i].group && all_trips[i].group->trips.size() > 1) sharingBeforeReshare++;
	}
}
	
bool pointee_is_equal(Trip& t1, Trip* t2)
{
	return &t1 == t2;
}

void generateLeaders(bool clearAfter = false)
{
	int actualSharing2 = 0;
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.group != NULL)
		{
			if (t1.group->trips.size() > 1)
			{
				actualSharing++;//maybe try deleting the group after adding trips.size to ensure we've added exactly all the groups once
				if (t1.group->leader == &t1)
				{
					actualSharing2 += t1.group->trips.size();
					groups++;
				}
			}
			//if (find(t1.group->trips.begin(), t1.group->trips.end(), &t1) == t1.group->trips.end())
			//list<Trip*>::iterator matching_iter = find_if(t1.group->trips.begin(), t1.group->trips.end(), bind1st(pointee_is_equal, &t1))
				//cout << t1id << " was not in its actualsharing set." << endl;
		}
	}
	/*
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
	}*/
	cout << "1: " << actualSharing << "   2: " << actualSharing2 << endl;

	if (clearAfter)
	{
		actualSharing = 0;
		groups = 0;
	}
}

//Attempts to re-share trips after tour-level removals
int reshared = 0;
void shareTrips2()
{

	int lastReshared = -1;
	while (reshared != lastReshared)
	{
		if (lastReshared >= 0)
			cout << reshared - lastReshared << " added" << endl;
		lastReshared = reshared;
		Timer ti("Resharing trips");
		for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
		{
			Trip& t1 = all_trips[t1id];
			if (t1.group == NULL)
			{
				for (int t2id : t1.potentialSharing)
				{
					Trip& t2 = all_trips[t2id];
					if (t2.group && t2.group->canAddTrip(t1))
					{
						t2.group->addTrip(t1, false);
						reshared++;
						break;
					}
				}
			}
		}
		for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
		{
			Trip& t1 = all_trips[t1id];
			if (t1.group == NULL)
			{
				formGroup(t1);
				if (t1.group && t1.group->trips.size() > 1)
				{
					reshared++;
				}
			}
		}
	}

	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.group == NULL)
		{
			t1.group = new VGroup(t1);
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

		if (t.group)
		{
			if (t.group->trips.size() > 1)
			{
				//actualSharing++;
				for (Trip* t2 : t.group->trips)
				{
					if (DrivingModes[t2->mode] && t2->group->leader != t2)
					{
						VMTReduction += distanceBetween(t2->origin, t2->destination);
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
		if (all_trips[t1id].group)
		{
			for (Trip* t2 : all_trips[t1id].group->trips)
				outf << ',' << t2->id;
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

	ofstream outf(NEW_TRIP_FILE);
	
	int sharedCount = 0;
	int unsharedCount = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.group)
		{
			if (t.group->trips.size() > 1)
			{
				//sharedCount++;
				if (t.group->leader == &t)
				{
					sharedCount += t.group->trips.size();// += t.actualSharing->size();
					int numPassengers = 0;
					for (Trip* t2 : t.group->trips)
					{
						numPassengers += t2->numPassengers;
					}
					outf << lineModify(lines[i], to_string(numPassengers), "5") << endl;
				}
			}
			else
			{
				unsharedCount++;
				outf << lines[i] << endl;
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
	//generateLeaders();
	checkTours();	
	shareTrips2();
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

