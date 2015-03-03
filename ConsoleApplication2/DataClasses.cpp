#include "stdafx.h"
#include "DataClasses.h"
#include "FastRand.h"
#include "Globals.h"
#include "MiscFunctions.h"

#include <map>
#include <vector>
#include <unordered_set>
#include <iostream>
using namespace std;

void reCheckTour(Tour& to);
void checkTour(Tour& to);

Trip::Trip()
{
	group = NULL;
	potentialSharing.reserve(4);
	shareable = -1;
	shared = 1;
	doable = false;
}

//Returns true if trip passes initial parameter checks
bool Trip::isShareable()
{

	if (shareable == -1)
		shareable = (
		all_people[perid].tours[tourid]->numStops <= MaxNumStops &&		//Number of stops
		distanceBetween(origin, destination) > MinDistanceTraveled &&	//Trip length
		all_people[perid].income < MaxIncome &&							//Income level
		TripModes[mode] == 1 &&											//Mode is valid check  (array index of the mode must be 1)
		((fastrand() % 100) + 1) > RandomFailChance &&					//Random chance (random int must be greater than fail chance)
		TripPurposes.find(purpose) != TripPurposes.end()				//Trip purpose (purpose must be in the set of allowed purposes)
		);
	return shareable;

}

//Sets a trip to be doable or not, recursively following any additions/subtractions
void Trip::setDoable(bool set, bool recheckTour)
{

	if (set == true && doable == false) //if we're making the trip doable
	{
		doable = true;
		if (recheckTour)
		{
			Tour& to = *all_people[perid].tours[tourid];
			reCheckTour(to);
		}
	}
	else if (set == false && doable == true)//if we're making the trip not doable
	{
		if (!DoableTripModes[mode])
		{
			doable = false;
			if (recheckTour)
			{
				Tour& to = *all_people[perid].tours[tourid];
				checkTour(to);
			}
		}
	}
}




Tour::Tour()
{
	trips.reserve(5);
	shared = 1;
}

Person::Person()
{
}