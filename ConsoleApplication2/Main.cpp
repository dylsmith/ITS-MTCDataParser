#include "stdafx.h"

#include "Globals.h"
#include "DataClasses.h"
#include "FastRand.h"
#include "MiscFunctions.h"
#include "QuickParser.h"
#include "QuickParser_inline.cpp"
#include "Timer.h"
#include "LoadData.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <sstream>
#include "VariableLoader.h"

using namespace std;

typedef vector<Trip*> d0;
typedef vector<d0> d1;
typedef vector<d1> d2;
typedef vector<d2> d3;



//Reserves space for all data
void reserveSpace()
{
	Timer t("Reserving space for all objects");

	memset(close, 0, DISTANCE_FILE_SIZE);
	dist = new float[DISTANCE_FILE_SIZE];
	all_households = new Household[HOUSEHOLD_FILE_SIZE+1];
	//all_people.reserve(PERSON_FILE_SIZE + 1);
	//Person* peopleSet = new Person[PERSON_FILE_SIZE + 1];
	//for (int i = 1; i <= PERSON_FILE_SIZE; i++)
	//	all_people[i] = &peopleSet[i];
	//all_people[0] = NULL;
	all_people = new Person[PERSON_FILE_SIZE + 1];
	all_tours = new Tour[TOUR_FILE_SIZE];
	all_trips = new Trip[TRIP_FILE_SIZE];
	closePoints = new vector<short>[NUM_LOCATIONS + 1];

	
	organized = new vector<Trip*>[1455 * 1455 * 24]();
	//organized = new vector<Trip*>[size]();//new d3(24*60, d2(1455, d1(1455, d0())));

	all_joint_tours = new Tour[JOINT_TOURS_FILE_SIZE];
	all_joint_trips = new Trip[JOINT_TRIPS_FILE_SIZE];
}

//Frees space
void cleanUp()
{
	free((void*)close);
	delete organized;
}

void EVCheck()
{
	//Check each person's conditions in each household
	for (int i = 1; i <= HOUSEHOLD_FILE_SIZE; i++)
	{
		Household& hh = all_households[i];
		for (auto& topair : hh.tours) //For each joint tour
		{
			Tour& to = *topair.second;
			for (Trip* t : to.trips) //For each joint trip
			{
				if (DrivingModes[t->mode]) //If trip is driving, add its distance to joint miles
				{
					hh.jointMilesDriven += distanceBetween(t->origin, t->destination);
				}
			}
		}
		
		if (hh.jointMilesDriven > EVAverageRange) //if joint miles driven is too far, house is not viable
		{
			hh.viable = false;
		}

		for (Person* p : hh.people) //For each person in household
		{
			for (auto& topair : p->tours) //For each tour
			{
				Tour& to = *topair.second;				
				for (Trip*& t1 : to.trips)	//For each trip
				{
					if (DrivingModes[t1->mode]) //If trip is driving, add distance driven to person's miles
					{
						p->milesDriven += distanceBetween(t1->origin, t1->destination);; //Sum up how far the person travels
					}
				}
			}

			hh.indivMilesDriven += p->milesDriven; //Add person's miles driven to total household miles driven

			if (p->milesDriven > EVAverageRange)	//If the person travels too far, household is not shared
			{
				hh.viable = false;
			}
		}

		if (hh.indivMilesDriven + hh.jointMilesDriven > hh.autos * EVAverageRange) //check if entire household drives too far
		{
			hh.viable = false;
		}
	} //for each household

	int total = 0;
	for (int i = 1; i <= HOUSEHOLD_FILE_SIZE; i++) //Sum up total households vs viable households
	{											
		Household& hh = all_households[i];
		if (hh.viable)
			++viableHouseholds;
	}


	vector<string> householdFile;
	householdFile.reserve(HOUSEHOLD_FILE_SIZE+1);

	ifstream inf(HOUSEHOLD_FILE);
	string header;
	getline(inf, header);
	header += ", EVViability";

	ofstream outf(HOUSEHOLD_EV_FILE);
	outf << header << endl;

	string line;
	for (int i = 1; i < HOUSEHOLD_FILE_SIZE; i++)
	{
		Household& hh = all_households[i];
		getline(inf, line);
		stringstream ss(line);
		int hhid;
		ss >> hhid;
		householdFile[hhid] = line;
	}
	
	for (int i = 1; i <= HOUSEHOLD_FILE_SIZE; i++)
	{
		Household& hh = all_households[i];

		outf << householdFile[hh.hhid]  << ", " << hh.viable << endl;
	}

	cout << viableHouseholds << "/" << HOUSEHOLD_FILE_SIZE << " households were viable, or " << setprecision(5) << (double)viableHouseholds / HOUSEHOLD_FILE_SIZE * 100 << "%." << endl;

}

struct
{
	bool operator()(const int lhs, const int rhs)
	{
		return all_people[lhs].totalScore < all_people[rhs].totalScore;
	}
} PotentialSharingSort;

//Compares trips, assuming they're potentially shareable already
inline bool compareTrips(Trip& trip1, Trip& trip2)
{
	return (trip1.perid != trip2.perid &&
		abs(trip1.minute - trip2.minute) <= MaxSharingTimeDifference);// &&
	//distanceBetween(trip1.origin, trip2.origin) < CLOSE_DISTANCE &&
	//distanceBetween(trip1.destination, trip2.destination) < CLOSE_DISTANCE);
}

void findPotentialSharing(Trip &t1)
{	
	//for (int minute = t1.minute - MaxSharingTimeDifference; minute <= t1.minute + MaxSharingTimeDifference; minute++)
	//{
		//if (minute < 0 || minute >= 24 * 60)
			//continue;
	for (int hour = t1.hour - 1; hour <= t1.hour + 1; hour++)
	{
		if (hour < 0 || hour >= 24)
			continue;
		for (int closeOrigin : closePoints[t1.origin])
		{
			for (int closeDestination : closePoints[t1.destination])
			{
				for (Trip* t2 : *sortedTrips(hour, closeOrigin, closeDestination))
				{
					if (compareTrips(t1, *t2))
					{			
						t1.potentialSharing->push_back(t2->id);
					}//if trips match up
				}//for each trip
			}//for each destination
		}//for each origin
	}//for each hour
}

//Parses the sorted trips, builds potential sharing lists
void analyzeTrips()
{
	if (largeCalculations)
		return;
	//For each trip, add other trips with the same hour and similar OD pairs to potential sharing
	Timer ct("Analyzing trips");

//#pragma omp parallel for
	int mod = TRIP_FILE_SIZE / 200;
	int count = 0;
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		if (t1id % mod == 0) cout << ++count << endl;
		findPotentialSharing(all_trips[t1id]);
	}
}

//Tries to form a new group around a trip
void formGroup(Trip& t1, int tid = -1)	//Returns true if at least one other trip is added. Will always create t1.actualSharing of some size >= 1
{
	if (DrivingModes[t1.mode] && t1.group == NULL && all_people[t1.perid].totalScore >= sharingRequirement)
	{
		t1.group = new VGroup(t1); //Give it a group, and add any potentially shared trips that can share with its group

		/*
		if (largeCalculations && t1.potentialSharing == NULL)
		{
			t1.potentialSharing = new vector<int>();
			t1.potentialSharing->reserve(130);
			findPotentialSharing(t1);
		}*/
		if (largeCalculations)
		{
			for (int hour = t1.hour - 1; hour <= t1.hour + 1; hour++)
			{
				if (hour < 0 || hour >= 24)
					continue;
				for (int closeOrigin : closePoints[t1.origin])
				{
					for (int closeDestination : closePoints[t1.destination])
					{
						for (Trip* t2 : *sortedTrips(hour, closeOrigin, closeDestination))
						{
							if (t1.group->canAddTrip(*t2))
							{
								t1.group->addTrip(*t2);
							}
						}//for each trip
					}//for each destination
				}//for each origin
			}//for each hour
		}
		else
		{
			for (int t2id : *t1.potentialSharing)
			{
				Trip& t2 = all_trips[t2id];
				if (t1.group->canAddTrip(t2))
				{
					t1.group->addTrip(t2);
				}
			}
		}

		/*
		if (largeCalculations && t1.potentialSharing != NULL)
		{
			delete t1.potentialSharing;
		}*/
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

int parallelShareTripsDone = 0;
mutex shareTripsCounter;
const int numThreads = 10;
void shareTripsThread(int tid)
{
	int count = 0;
	int step = 100000;
	for (int t1id = tid; t1id < TRIP_FILE_SIZE; t1id += numThreads)
	{
		if (++count == step)
		{
			count = 0;
			shareTripsCounter.lock();
			parallelShareTripsDone += step;
			shareTripsCounter.unlock();
			cout << parallelShareTripsDone << "/" << TRIP_FILE_SIZE << " thread id: " << tid << endl;
		}
		formGroup(all_trips[t1id], tid);
	}
}


//Tries to form actual sharing groups for drivers
void shareTrips()
{
	Timer ti("Sharing trips");

	time_t start = time(0);
	//Try to form a group for all trips
	int done = 0;
	int step = 100000;
	int count = 0;

	//#pragma omp parallel for firstprivate(count)
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		if (++count == step)
		{
			count = 0;
			done += step;

			time_t end = time(0);
			int tg = end - start;
			if (tg < 1) tg = 1;
			int secondsleft = (TRIP_FILE_SIZE - t1id) / (done / tg);
			int minutesleft = secondsleft / 60;
			secondsleft %= 60;

			cout << done << "/" << TRIP_FILE_SIZE << "  " << minutesleft << " minutes " << secondsleft << " seconds left." << endl;
		}
		formGroup(all_trips[t1id]);
	}
	
#pragma omp parallel for reduction(+:sharingBeforeTourLevel)
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t1 = all_trips[t1id];
		if (t1.group == NULL) //If group could not be formed, give it a solo group
			t1.group = new VGroup(t1);
		else if (t1.group->trips.size() > 1) //if group was formed and is sharing with others
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
				/*
				if (largeCalculations && t1.potentialSharing == NULL)
				{
					t1.potentialSharing = new vector<int>();
					t1.potentialSharing->reserve(130);
					findPotentialSharing(t1);
				}*/

				if (largeCalculations)
				{
					for (int hour = t1.hour - 1; hour <= t1.hour + 1; hour++)
					{
						if (hour < 0 || hour >= 24)
							continue;
						for (int closeOrigin : closePoints[t1.origin])
						{
							for (int closeDestination : closePoints[t1.destination])
							{
								for (Trip* t2 : *sortedTrips(hour, closeOrigin, closeDestination))
								{
									if (t2->group && t2->group->canAddTrip(t1)) //if t2 has a group and can accept t1
									{
										t2->group->addTrip(t1); //add t1 to t2's group
										reshared++;
										break;
									}
								}//for each trip
							}//for each destination
						}//for each origin
					}//for each hour
				}
				else
				{
					for (int t2id : *t1.potentialSharing) //for each trip t1 might share with
					{
						Trip& t2 = all_trips[t2id];
						if (t2.group && t2.group->canAddTrip(t1)) //if t2 has a group and can accept t1
						{
							t2.group->addTrip(t1); //add t1 to t2's group
							reshared++;
							break;
						}
					}
				}

				/*
				if (largeCalculations && t1.potentialSharing != NULL)
				{
					delete t1.potentialSharing;
				}*/
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
	double totalDrivenMiles = 0;
	for (int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		Trip& t = all_trips[t1id];
		if (DrivingModes[t.mode])
			totalDrivenMiles += distanceBetween(t.origin, t.destination);

		totalMiles += distanceBetween(t.origin, t.destination);
		if (t.group && t.group->leader == &t && t.group->trips.size() > 1) //if t is a leader and sharing with others
		{
			int driving = 0;
			for (Trip* t2 : t.group->trips)
			{
				if (t2->group->leader != t2 && DrivingModes[t2->mode])
				{
					driving++;
					miles += distanceBetween(t2->origin, t2->destination);
				}
			}
			miles2 += driving * distanceBetween(t.origin, t.destination);
		}
	}
	cout << "Total miles traveled by all trips: " << totalMiles << endl;
	cout << "Total miles traveled by all drivers: " << totalDrivenMiles << endl;
	cout << "VMT, by driving passenger's distance saved: " << miles << endl;
	cout << "VMT, by leader's distance * number of driving passengers: " << miles2 << endl;
}

//Gathers data points after algorithms are finished
void postStatistics()
{
	Timer ti("Writing output.txt");

	int actualSharing2 = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.potentialSharing && t.potentialSharing->size() > 0)	
			potentialSharing++;

		if (t.group) //if t has a group
		{
			if (t.group->trips.size() > 1) //if t is sharing with others
			{
				actualSharing++;
				if (t.group->leader == &t)
				{
					for (Trip* t2 : t.group->trips) //for each shared trip
					{
						if (DrivingModes[t2->mode] && t2->group->leader != t2) //If t2 is a driver and not a leader
						{
							VMTReduction += distanceBetween(t2->origin, t2->destination); //Add its distance to saved VMT count
						}
					}
					actualSharing2 += t.group->trips.size();
					groups++;
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

	outf << "Average household income: " << householdIncome << endl;
	outf << "Average household # vehicles: " << householdVehicles << endl;

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
	outf << "perid, tourid, tripid, numpassengers, sharing set" << endl;
	for(int t1id = 0; t1id < TRIP_FILE_SIZE; t1id++)
	{
		//Write perid, tourid, tripid, numPassengers, actual sharing set (separated by spaces)
		Trip& t1 = all_trips[t1id];
		outf << t1.perid << ", " << t1.tourid << ", " << t1.id << ", ";
		if (all_trips[t1id].group)
		{
			outf << t1.group->trips.size() << ",";
			for (Trip* t2 : all_trips[t1id].group->trips)
				outf << ' ' << t2->id;
		}
		else
		{
			outf << "1, " << t1id << endl;
		}
		outf << endl;
	}
}

void writeTripLine(string& line, ofstream& outf, Trip& t)
{
	if (t.group)
	{
		if (t.group->trips.size() > 1)
		{
			outf << lineModify(line, "5") << endl;
		}
		else
		{
			outf << line << endl;
		}
	}
	else
	{
		cout << "Orphaned trip!" << endl;
	}
}

//Writes trip details out to files
void tripDetailsOutput()
{
	Timer ti("Writing trip details");

	string* lines;
	lines = new string[TRIP_FILE_SIZE];

	string line;
	string header;

	ifstream inf(TRIP_FILE);
	getline(inf, header);	///Read Header

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		getline(inf, line);	//Read all lines of the file into memory
		lines[i] = line;
	}

	if (WriteTripDetails)
	{
		ofstream both(TRIP_DETAILS_FILE);
		ofstream shared(SHARED_DETAILS_FILE);
		ofstream unshared(UNSHARED_DETAILS_FILE);
		both << header << endl;
		shared << header << endl;
		unshared << header << endl;

		for (int i = 0; i < TRIP_FILE_SIZE; i++)
		{
			Trip& t = all_trips[i];
			if (t.group)	//If t has a group (all should)
			{
				if (t.group->trips.size() > 1)	//If t is sharing with others
				{
					if (t.group->leader == &t)	//If t is a leader
					{
						string line = lineModify(lines[i], "5");//Write line to file, changing trip mode to 5
						both << line << endl;
						shared << line << endl;
					}
				}
				else //if trip is not sharing with others
				{
					both << lines[i] << endl;	//Write line to file normally
					unshared << lines[i] << endl;
				}
			} //if t.group
		} //for each trip
	}

	cout << "Opening " << ALL_TRIP_DETAILS_FILE << endl;
	ofstream all(ALL_TRIP_DETAILS_FILE);
	all << header << endl;
	int noGroup = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.group)
		{
			if (t.group->trips.size() > 1) //If trip is in a sharing group, change its mode and output its details
			{
				all << lineModify(lines[i], "5") << endl;
			}
			else //Otherwise just output its details
			{
				all << lines[i] << endl;
			}
		}
	}
}

/*
for each person
	record %of shared trips

for each household
	count number of joint trips each person takes, add them to person

build a functor to compare people based on % shared trips and # joint trips


Beforehand:
find households with no shared trips or 1 member, these may be freely removed (so remove people based on % trips here)
*/

struct
{
	bool operator()(const Person* lhs, const Person* rhs)
	{
		return rhs->totalScore < lhs->totalScore;
	}
} PeopleSort;


int PeopleCompare(const void* a, const void* b)
{
	double s1 = ((Person*)a)->totalScore;
	double s2 = ((Person*)b)->totalScore;
	if (s1 == s2)
		return 0;
	else if (s1 < s2)
		return 1;
	else
		return -1;
}

void writePeopleFile(unordered_set<int> &sharedPeople)
{
	Timer ti("Writing shared/unshared files");
	vector<string> peopleFile;
	peopleFile.reserve(PERSON_FILE_SIZE);
	ifstream inf(PERSON_FILE);
	string header;
	getline(inf, header);
	header += ", Solo Sharing Chance, Household Sharing Chance, Total Sharing Chance";
	
	ofstream sharedPeopleFile(SHARED_PERSON_FILE);
	ofstream unsharedPeopleFile(UNSHARED_PERSON_FILE);

	sharedPeopleFile << header << endl;
	unsharedPeopleFile << header << endl;

	string line;
	for (int i = 1; i <= PERSON_FILE_SIZE; i++)
	{
		getline(inf, line);
		stringstream ss(line);
		int hhid, perid;
		ss >> hhid;
		while (ss.peek() == ',' || ss.peek() == ' ') ss.ignore();
		ss >> perid;

		Person& p = all_people[perid];

		if (sharedPeople.find(perid) == sharedPeople.end())
		{	//TODO: merge these into one sstream before output
			unsharedPeopleFile << line << ", " << p.rideShareProb << ", " << p.householdInteractionProb << ", " << p.totalScore << endl;
		}
		else
		{
			sharedPeopleFile   << line << ", " << p.rideShareProb << ", " << p.householdInteractionProb << ", " << p.totalScore << endl;
		}
	}
}

double rideShareProbability(Person& p)
{
	double totalTripCount = 0;
	double sharedTripCount = 0;

	for (auto& topair : p.tours)
	{
		Tour* to = topair.second;
		for (Trip* t : to->trips)
		{
			double tripCost = t->mandatory ? mandatoryTripWeight : nonMandatoryTripWeight;
			totalTripCount += tripCost;
			if (t->group && t->group->trips.size() > 1)
				sharedTripCount += tripCost;
		}
	}
	if (totalTripCount == 0) return 0;
	return sharedTripCount / totalTripCount * 100;
}

double householdInteractionProbability(Person& p)
{
	int householdInteractionProb = 0;
	Household& hh = all_households[p.hhid];
	if (hh.tours.size() > 0)
	{
		for (auto& topair : hh.tours)
		{
			Tour* to = topair.second;
			for (Trip* t : to->trips)
				++householdInteractionProb;
		}
	}
	return householdInteractionProb;
}

void tryToGroup(Trip& t1)
{
	if (!t1.group)
		t1.group = new VGroup(t1); //Give it a group, and add any potentially shared trips that can share with its group


	/*
	if (largeCalculations && t1.potentialSharing == NULL)
	{
		t1.potentialSharing = new vector<int>();
		t1.potentialSharing->reserve(130);
		findPotentialSharing(t1);
	}*/

	if (largeCalculations)
	{
		for (int hour = t1.hour - 1; hour <= t1.hour + 1; hour++)
		{
			if (hour < 0 || hour >= 24)
				continue;
			for (int closeOrigin : closePoints[t1.origin])
			{
				for (int closeDestination : closePoints[t1.destination])
				{
					for (Trip* t2 : *sortedTrips(hour, closeOrigin, closeDestination))
					{
						if (t1.group->canAddTrip(*t2))
						{
							t1.group->addTrip(*t2);
						}
					}//for each trip
				}//for each destination
			}//for each origin
		}//for each hour
	}
	else
	{
		for (int t2id : *t1.potentialSharing)
		{
			Trip& t2 = all_trips[t2id];
			if (t1.group->canAddTrip(t2))
			{
				t1.group->addTrip(t2);
			}
		}
	}

	/*
	if (largeCalculations && t1.potentialSharing != NULL)
	{
		delete t1.potentialSharing;
	}*/

	if (t1.group->trips.size() < 2)
	{
		delete t1.group;
	}
}

void peopleOutput()
{
	Timer* ti = new Timer("Generating sharing scores");
	double highestScore = 0;
	double scoreSum = 0;
	double scoreCount = 0;
	for (int i = 1; i <= PERSON_FILE_SIZE; i++)
	{
		Person& p = all_people[i];
		p.rideShareProb = rideShareProbability(p);
		p.householdInteractionProb = householdInteractionProbability(p);
		p.totalScore = (p.rideShareProb * rideShareWeight) - (p.householdInteractionProb * householdInteractionWeight);
		if (p.totalScore > highestScore)
			highestScore = p.totalScore;

		++scoreCount;
		scoreSum += p.totalScore;
	}
	cout << "Average score: " << scoreSum / scoreCount << endl; 
	delete ti;

	/*
	ti = new Timer("Sorting people by sharing scores");
	sort(next(all_people.begin()), all_people.end(), PeopleSort);
	//sort(&all_people[1], &all_people[PERSON_FILE_SIZE], PeopleSort);
	//qsort((void*)(&all_people[1]), PERSON_FILE_SIZE, sizeof(Person), PeopleCompare);
	delete ti;*///

	ti = new Timer("Deleting trip groups");
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		all_trips[i].group = NULL;
		/*
		Trip& t = all_trips[i];
		if (t.group)
		{
			delete t.group;
		}*/

	}
	delete ti;

	ti = new Timer("Sorting people by total score");
	vector<Person*> personptrs;
	personptrs.reserve(PERSON_FILE_SIZE + 1);
	for (int i = 0; i <= PERSON_FILE_SIZE; i++)
		personptrs.push_back(&all_people[i]);

	sort(personptrs.begin() + 1, personptrs.end(), PeopleSort);
	delete ti;


	ti = new Timer("Sharing trips for highest scoring people");
	unordered_set<int> sharedPeople;

	int numTripsToShare = TRIP_FILE_SIZE * PercentTripsToShare;
	int totalSharedTrips = 0;

	/*
	for (int i = 1; i <= 20; i++)
		cout << personptrs[i]->totalScore << endl;
	cout << endl;
	for (int i = PERSON_FILE_SIZE - 20; i <= PERSON_FILE_SIZE; i++)
		cout << personptrs[i]->totalScore << endl;*/

	sharingRequirement = highestScore;
	int prevSharedTrips = 0;
	while (totalSharedTrips < numTripsToShare)
	{
		int peopleAdded = 0;
		for (int i = 1; i <= PERSON_FILE_SIZE; i++)
		{
			Person& p = *personptrs[i];
			if (p.totalScore < sharingRequirement)
			{
				continue;
			}
			else
			{
				for (auto& topair : p.tours)
				{
					Tour* to = topair.second;
					for (Trip* t1 : to->trips)
					{
						//if (DrivingModes[t1.mode] && t1.group == NULL && all_people[t1.perid]->totalScore >= sharingRequirement)
						if (DrivingModes[t1->mode])
						{
							tryToGroup(*t1);
							if (t1->group && t1->group->trips.size() > 1)
							{
								for (Trip* groupedTrip : t1->group->trips)
								{
									if (sharedPeople.find(groupedTrip->perid) == sharedPeople.end())
									{
										peopleAdded++;
										sharedPeople.insert(groupedTrip->perid);
									}
								}
								totalSharedTrips += t1->group->trips.size();
								if (totalSharedTrips >= numTripsToShare)
								{
									delete ti; 
									cout << ">=" << sharingRequirement << ": " << endl << "Total shared trips: " << totalSharedTrips << "/" << numTripsToShare << endl << "  Trips added: " << totalSharedTrips - prevSharedTrips << endl << "  People added: " << peopleAdded << endl;
									writePeopleFile(sharedPeople);
									return;
								}
							}
						}
					}
				}
			}
		}
		cout << ">=" << sharingRequirement << ": " << endl << "Total shared trips: " << totalSharedTrips << "/" << numTripsToShare << endl << "  Trips added: " << totalSharedTrips - prevSharedTrips << endl << "  People added: " << peopleAdded << endl;

		prevSharedTrips = totalSharedTrips;
		totalSharedTrips = 0;
		sharingRequirement -= sharingRequirementStep;

		if (peopleAdded == 0)
		{
			break;
		}
	}

	cout << "This part of peopleOutput() should not be reached!" << endl;
	/*
	delete ti;
	writePeopleFile(sharedPeople);
	return;*/

}

//Records total execution time
void timerWrapper()
{
	
	Timer total("Total");
	cout << tod() << "Starting." << endl;
	load();

	QuickParser q(HOUSEHOLD_FILE);

	if (ExecutionMode == 0) //if ridesharing
	{
		reserveSpace();


		departprobs = new DepartProbability();
		parseClosePoints();
		parseHouseholds();

		parsePeople();
		parseTours();
		parseJointTours();
		parseTrips();
		parseJointTrips();



		analyzeTrips(); // Generates potential s haring lists
		shareTrips(); //Actually shares trips into groups 
		checkTours(); //Removes trips from tours with <% sharing
		shareTrips2(); //Reshares orphaned trips

		countMiles(); //Calculates VMT
		postStatistics();	//DataPoints.txt - shared trip counts, etc.

		if (WriteTripDetails)
			tripDetailsOutput(); // TripsOutput.txt - each trip's full details, after sharing
							//Changes all group leaders to mode 5
							//Writes all trips to one file, all shared trips to one file, and all shared trips to a file

		if (WriteTripSharing)
			tripSharingOutput(); //TripSharing.txt - each trip's actual sharing list

		if (WriteInducedDemand)
			peopleOutput();
	}
	else if (ExecutionMode == 1) //if EV
	{
		departprobs = new DepartProbability();
		reserveSpace();
		parseDistances();
		parseHouseholds();
		parsePeople();
		parseTours();
		parseTrips();
		parseJointTours();
		parseJointTrips();
		EVCheck();
		ofstream outf(DATA_FILE);
		outf << "EV Algorithm viable households: " << viableHouseholds << "/" << HOUSEHOLD_FILE_SIZE 
			 << " households were viable, or " << setprecision(5) << (double)viableHouseholds / HOUSEHOLD_FILE_SIZE * 100 << "%." << endl;
	}
	else if (ExecutionMode == 2)
	{
		load();
	}
}

struct PersonCompare
{
	inline bool operator()(const Person* p1, const Person* p2)
	{
		return p1->income < p2->income;
	}
};
#include <algorithm>

vector<Person*> sortPeople()
{
	Timer t("Sorting people list by ride-sharing probability");
	vector<Person*> people;
	for (int i = 0; i < PERSON_FILE_SIZE; i++)
	{
		people.push_back(&all_people[i]);
	}

	sort(people.begin(), people.end(), PersonCompare());
}
//Main
int _tmain(int argc, _TCHAR* argv[])
{
	path = argv[0];
	timerWrapper();
	cout << "Finished." << endl;
	pause();

	return 0;
}


