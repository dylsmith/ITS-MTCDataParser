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
		for (int k = i + 1; k <= NUM_LOCATIONS; k++)
		{

			closePoints[i].push_back(i);
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
			organized[trip.hour][trip.origin][trip.destination].push_back(&all_trips[i]);

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

void removeTrip(Trip& trip)
{
	trip.sharingList.clear();
	for (int otherTrip : trip.sharingList)
	{
		remove(all_trips[otherTrip].sharingList, trip.id);
	}
}

void shareTrips()
{
	Timer ti("Sharing trips");
	for (int tripNum = 0; tripNum < TRIP_FILE_SIZE; tripNum++)
	{
		if (tripNum % 1000000 == 0) cout << "Sharing trips: " << (long double)tripNum / TRIP_FILE_SIZE * 100 << "%" << endl;
		Trip& trip = all_trips[tripNum];
		if (trip.shareable != BEING_SHARED)
		{
			for (int nextTripID : trip.sharingList)
			{
				Trip& nextTrip = all_trips[nextTripID];
				if (nextTrip.shareable != BEING_SHARED && trip.numPassengers + nextTrip.numPassengers <= MaxPeople)
				{
					trip.shareable = BEING_SHARED;
					nextTrip.shareable = BEING_SHARED;
					trip.actualSharing.push_back(nextTripID);
					trip.numPassengers += nextTrip.numPassengers;
				}
			}
			/*
			//cout << "For trip " << trip.id << endl;
			//for (int i : trip.sharingList) cout << "  " << i << endl;
			while (trip.sharingList.size() > 0)
			{
				//int nextTripNum = fastrand() % trip.sharingList.size();
				Trip& nextTrip = all_trips[trip.sharingList[fastrand() % trip.sharingList.size()]];
				if (nextTrip.shareable != BEING_SHARED && trip.numPassengers + nextTrip.numPassengers <= MaxPeople)
				{
					//removeTrip(nextTrip);	//Removes this trip from every trip that thinks it can share with it
					trip.actualSharing.push_back(nextTrip.id);
					trip.shareable = BEING_SHARED;
					nextTrip.shareable = BEING_SHARED;
					//nextTrip.actualSharing.push_back(trip.id);
					trip.numPassengers += nextTrip.numPassengers;


				}
				remove(trip.sharingList, nextTrip.id);*/

				/*
				else
				{
				//cout << "Removing " << tripNum << " from " << nextTrip.id << endl;
				//for (int j : nextTrip.sharingList) cout << j << " "; cout << endl;
				//remove(nextTrip.sharingList, tripNum);
				//cout << "Removing " << nextTrip.id << " from " << tripNum << endl;
				remove(trip.sharingList, nextTrip.id);
				}*/
			//}
		}
	}
}

void checkTours()
{
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		Tour& tour = all_tours[i];
		int doableTrips = 0;
		for (Trip*& trip : tour.trips)
		{
			if (trip->isShareable() || DoableTripModes[trip->mode])
			{

			}
		}
	}
}

inline bool compareTrips(Trip& trip1, Trip& trip2)
{
	return (trip1.perid != trip2.perid 
		//find(closePoints[trip1.destination].begin(), closePoints[trip1.destination].end(), trip2.destination) != closePoints[trip1.destination].end()
		);
}

/*
void analyzeTrips()
{
	Timer ct("Comparing trips");

	long int sharedtrips = 0;
	long long int total = 0;
	for (int hour = 0; hour < 24; hour++)
	{
		for (int origin = 1; origin <= NUM_LOCATIONS; origin++)
		{
			for (Trip* trip1 : organized[hour][origin])
			{
				for (int closePoint : closePoints[origin])
				{
					for (Trip* trip2 : organized[hour][closePoint])
					{
						if (++total % 100000000 == 0) cout << (double)origin / NUM_LOCATIONS << ": " << total << endl;
						if (compareTrips(*trip1, *trip2))
						{
							trip1->sharingList.push_back(trip2->id);
							sharedtrips++;
						}
					}
				}
			}
		}
	}

	cout << (((long double)sharedtrips / TRIP_FILE_SIZE) * 100) << "% of trips were shared." << endl;
}*/

void analyzeTrips()
{
	Timer ct("Comparing trips");

	long long int sharedtrips = 0;
	long long int total = 0;
	for (int hour = 0; hour < 24; hour++)
	{
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
								if (++total % 100000000 == 0) cout << "Comparing trips: " << (((double)origin + (NUM_LOCATIONS * hour)) / (NUM_LOCATIONS * 24)) * 100 << "%" << endl;
								if (compareTrips(*trip1, *trip2))
								{
									if (trip2->id < 0)
										cout << trip2->id << endl;
									trip1->sharingList.push_back(trip2->id);
									sharedtrips++;
								}
							}
						}
					}
				}
			}
		}
	}

	cout << (((long double)sharedtrips / TRIP_FILE_SIZE) * 100) << "% of trips were shared." << endl;
}

void averageSharedTrips()
{
	Timer ti("Counting average number of shared trips");
	long int totalshared = 0;
	long double milesSaved = 0;
	long int ridesSaved = 0;
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		int driversSaved = all_trips[i].actualSharing.size();
		totalshared += driversSaved;
		milesSaved += (double)driversSaved * distanceBetween2(all_trips[i].origin, all_trips[i].destination);
	}

	
	cout << "Each trip was actually shared with an average of " << (double)totalshared / TRIP_FILE_SIZE << " other trips." << endl;
	cout << "Saved " << milesSaved << " vehicle miles." << endl;
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

void OMPInfo()
{
	cout << "Max threads: " << omp_get_max_threads() << endl;
	cout << "Max processors: " << omp_get_num_procs() << endl;
	cout << "Nested parallelism: " << omp_get_nested() << endl;
}

void cleanUp()
{
	free((void*)close);
	delete[] dist;
	delete[] all_people;
	delete[] all_tours;
	delete[] all_trips;
	delete[] closePoints;
	for (int i = 0; i < 24; i++)
		for (int k = 1; k <= NUM_LOCATIONS; k++)
			delete organized[i][k];
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

	shareTrips();
	averageSharedTrips();


	for (int i = 0; i < 20; i++)
	{
		cout << i << ": " << endl;
		for (int sh : all_trips[i].sharingList)
		{
			cout << "  " << sh << endl;
		}
	}


}

int _tmain(int argc, _TCHAR* argv[])
{
	timerWrapper();
	
	pause();
	return 0;
}


