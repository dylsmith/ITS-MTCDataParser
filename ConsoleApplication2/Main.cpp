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
	//For each trip, add other trips with the same hour and similar OD pairs to potential sharing
	Timer ct("Analyzing trips");

	long long int sharedtrips = 0;
	for (int hour = 5; hour < 24; hour++)
	{
		cout << (double)(hour - 5) / 19 << "% done" << endl;
		for (int origin = 1; origin <= NUM_LOCATIONS; origin++)
		{
			for (int destination = 1; destination <= NUM_LOCATIONS; destination++)
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
	for (Trip* t : to.trips) //Count number of doable trips
		if (t->doable)
			doableTripCount++;

	//If tour is not shared, has trips, and has enough doable trips
	if (!to.shared && to.trips.size() > 0 && (((double)doableTripCount / to.trips.size()) >= TourDoableRequirement))
	{
		to.shared = 1;
		//For each trip in the tour, if it's not shared  and is shareable, share it
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
	if (t1.group == NULL) //if trip doesn't have a group yet
	{
		for (int t2id : t1.potentialSharing) //for each potentially shared trip
		{
			Trip& t2 = all_trips[t2id];
			if (t2.group && t2.group->canAddTrip(t1)) //if trip can share with t2's group, add it
			{
				t2.group->addTrip(t1, false);
				return;
			}
		}

		//If group is still null, give it a solo group
		if (t1.group == NULL)
			t1.group = new VGroup(t1);

		//if trip is driving, try to add each potentialsharing group to it, if possible
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
	//If tour doesn't have a group yet and is a driver
	if (DrivingModes[t1.mode] && t1.group == NULL)
	{
		t1.group = new VGroup(t1); //Give it a group, and add any potentially shared trips that can share with its group
		for (int t2id : t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (t1.group->canAddTrip(t2)) 
				t1.group->addTrip(t2, false);
		}
	}
}

//Checks to see if a tour cannot be shared, and unshares it if needed
void checkTour(Tour& to)
{
	int doableTripCount = 0;
	for (Trip* t : to.trips) //Count how many trips are doable
		if (t->doable)
			doableTripCount++;

	//If the tour is shareable, has trips, and not enough trips are shared
	if (to.shared && to.trips.size() > 0 && (((double)doableTripCount / to.trips.size()) < TourDoableRequirement))
	{
		to.shared = 0;
		vector<Trip*>::iterator it = to.trips.begin();
		while (it != to.trips.end()) //While we haven't gone through the whole list
		{
			VGroup* group = (*it)->group; //Group is set to the trip's group

			if (group != NULL) //if group exists
			{
				int size = group->trips.size();
				group->removeTrip(**it);
				//if removing the trip affected the group's size, restart to make sure nothing is missed
				if (size != (*it)->group->trips.size()) 
					it = to.trips.begin();
				else
					it++;
			}
			else
				it++;
		}
	}
}

//Tries to form actual sharing groups for drivers
void shareTrips()
{
	Timer ti("Sharing trips");
	//Try to form a group for all trips
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
		formGroup(all_trips[t1id]);
	
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.group == NULL) //If group could not be formed, give it a solo group
			t1.group = new VGroup(t1);
		else if (t1.group->trips.size() > 1) //if group was formed and is shring with others
			sharingBeforeTourLevel++;
	}
}

//Checks to ensure all tours can be shared
void checkTours()
{
	Timer* ti = new Timer("Checking Tours");

	//Check the tour, ensuring it has enough doable trips. Remove all trips from sharing if not
	int prevUnshared = unshared;
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
		checkTour(all_tours[i]);

	//If we have unshared items
	if (prevUnshared != unshared)	//Repeat until convergence
	{
		delete ti;
		cout << unshared - prevUnshared << " unshared. Going again: " << endl;
		checkTours();
	}
	else //No new items were deleted
	{
		cout << unshared - prevUnshared << " unshared." << endl; 
		for (int i = 0; i < TRIP_FILE_SIZE; i++) //if t1 has a group and is sharing with others
			if (all_trips[i].group && all_trips[i].group->trips.size() > 1) sharingBeforeReshare++;
	}
}

//Attempts to re-share trips after tour-level removals
void shareTrips2()
{

	int reshared = 0; //Total reshared trip count
	int lastReshared = -1; //Reshared trip count before each iteration

	while (reshared != lastReshared) //While we have added new trips
	{
		if (lastReshared >= 0)
			cout << reshared - lastReshared << " added" << endl;
		lastReshared = reshared;

		Timer ti("Resharing trips");
		//Try to add the trip to an existing group
		for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
		{
			Trip& t1 = all_trips[t1id];
			if (t1.group == NULL) //if t1 has a group
			{
				for (int t2id : t1.potentialSharing) //for each trip t1 might share with
				{
					Trip& t2 = all_trips[t2id];
					if (t2.group && t2.group->canAddTrip(t1)) //if t2 has a group and can accept t1
					{
						t2.group->addTrip(t1, false); //add t1 to t2's group
						reshared++;
						break;
					}
				}
			}
		}
		
		//Form groups for the remaining trips
		for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
		{
			Trip& t1 = all_trips[t1id];
			if (t1.group == NULL) //if t1 doesn't have a group
			{
				formGroup(t1);
				if (t1.group && t1.group->trips.size() > 1) //if we gave it a group and it's sharing
					reshared++;
			}
		}
	}

	//Fill in any remaining groups with soo trips
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.group == NULL)
			t1.group = new VGroup(t1);
	}
}

void countMiles()
{
	double miles = 0;
	double miles2 = 0;
	double totalMiles = 0;
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t = all_trips[t1id];

		totalMiles += distanceBetween(t.origin, t.destination);
		miles += distanceBetween(t.origin, t.destination);
		if (t.group)
		{

			if (t.group->leader == &t)
			{
				int driving = 0;
				for (Trip* t2 : t.group->trips)
				{
					if (t2->group->leader != t2 && DrivingModes[t2->mode])
					{
						driving++;
					}
				}
				miles2 += driving * distanceBetween(t.origin, t.destination);
			}
		}
	}
	miles /= TRIP_FILE_SIZE;
	cout << "Total vehicle miles traveled (by all trips): " << totalMiles << endl;
	cout << "Average trip length: " << miles << endl;
	cout << "VMT, if all non-leaders would have driven the same length as the leader: " << miles2 << endl;
}

//Gathers data points after algorithms are finished
void postStatistics()
{
	Timer ti("Writing output.txt");

	int actualSharing2 = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.potentialSharing.size() > 0)	
			potentialSharing++;

		if (t.group) //if t has a group
		{
			if (t.group->trips.size() > 1) //if t is sharing with others
			{
				actualSharing++;
				if (t.group->leader == &t)
				{
					actualSharing2 += t.group->trips.size();
					groups++;
				}

				for (Trip* t2 : t.group->trips) //for each shared trip
				{
					if (DrivingModes[t2->mode] && t2->group->leader != t2) //If t2 is a driver and not a leader
					{
						//TODO: include leader's driving distance to pick up everyone
						VMTReduction += distanceBetween(t2->origin, t2->destination); //Add its distance to saved VMT count
					}
				}
			}
			else //if t is not sharing with others
			{
				solo++;
			}


		}
		else if (t.isShareable())	//if t does not have a group, but is shareable
		{
			orphaned++;
		}
	}

	ofstream outf(DATA_FILE);
	if (Maximize)
		outf << "Maximizing group size" << endl;
	else
		outf << "Minimizing group size" << endl;

	outf << "Total trips: " << TRIP_FILE_SIZE << endl;
	outf << "Shareable trips (precondition): " << shareable << endl;
	outf << "Trips with at least one potentially shared trip: " << potentialSharing << endl;
	outf << "Trips with at least one actually shared trip (before tour-level checks): " << sharingBeforeTourLevel << endl;
	outf << "Trips with at least one actually shared trip (before re-sharing): " << sharingBeforeReshare << endl;
	outf << "Final trips with at least one actually shared trip: " << actualSharing << endl;
	outf << "Sum of each leader's group size, if sharing: " << actualSharing2 << " (should be the same as above)" << endl;
	outf << "Trips with no actually shared trips: " << solo << endl;
	outf << "Solo trips + shared trips: " << solo + actualSharing << " (should equal " << TRIP_FILE_SIZE << ")" << endl;
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
			for (Trip* t2 : all_trips[t1id].group->trips)
				outf << ',' << t2->id;
		//TODO: add number of passengers here, second column
		//TODO: write perid, tourid, tripid, numpassengers, (actualy sharing set, separated by spaces)
	}
	outf << '\n';
}

//Writes trip details out to files
void tripDetailsOutput()
{
	Timer ti("Writing trip details");

	ifstream inf(TRIP_FILE);
	string* lines;
	lines = new string[TRIP_FILE_SIZE];
	string line;
	getline(inf, line);	///Read Header

	ofstream outf(TRIP_DETAILS_FILE);
	ofstream shared(SHARED_DETAILS_FILE);
	ofstream unshared(UNSHARED_DETAILS_FILE);
	outf << line << endl;
	shared << line << endl;
	unshared << line << endl;

	int count = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		getline(inf, line);	//Read all lines of the file into memory
		lines[count] = line;
		++count;
	}

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.group)	//If t has a group (all should)
		{
			if (t.group->trips.size() > 1)	//If t is sharing with others
			{
				if (t.group->leader == &t)	//If t is a leader
				{
					int numPassengers = 0;
					for (Trip* t2 : t.group->trips)	//Count its number of passengers
						numPassengers += t2->numPassengers;
					
					//Write line to file, changing trip size and mode to numPassengers and 5
					string line = lineModify(lines[i], to_string(numPassengers), "5");
					outf << line << endl;
					shared << line << endl;
				}
			}
			else //if trip is not sharing with others
			{
				outf << lines[i] << endl;	//Write line to file normally
				unshared << lines[i] << endl;
			}
		} //if t.group
	} //for each trip
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

	countMiles();
	postStatistics();	//DataPoints.txt - shared trip counts, etc.
	if (WriteTripDetails)
		tripDetailsOutput(); //TripsOutput.txt - each trip's full details, after sharing

	if (WriteTripSharing)
		tripSharingOutput(); //TripSharing.txt - each trip's actual sharing list
}

//Main
int _tmain(int argc, _TCHAR* argv[])
{
	
	timerWrapper();
	
	cout << "Finished." << endl;
	pause();
	return 0;
}


/*
print each person and their probability data

*/

