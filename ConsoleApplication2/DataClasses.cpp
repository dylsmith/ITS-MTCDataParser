#include "stdafx.h"
#include "DataClasses.h"
#include "FastRand.h"
#include "Globals.h"
#include "MiscFunctions.h"
#include "Timer.h"

#include <map>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <fstream>
using namespace std;

void checkTour(Tour& to);

//Generate a random minute offset, based on observed departure quartiles
int DepartProbability::generate(int zone, int hour)
{
	int county = zoneToCounty(zone);
	int probBlock = hourToBlock(hour);

	int rand = fastrand() % 100;
	int quartile = 0;
	vector<int>& prob = probs[county]; //probability set of interest
	for (int i = 0; i < 5; i++)
	{
		if (rand < prob[probBlock * 4 + quartile])	//go to the right set of 4 probabilities, then check if we belong in that quartile
			break;
		quartile++; //loop until we're in the right one
	} 

	return quartile * 15 + (fastrand() % 15); //return a random minute in that quartile
}

//Convert an hour to a probability block (chunks of hours)
int DepartProbability::hourToBlock(int hour)
{
	if (hour >= 3)
	{
		if (hour < 6)
		{
			return 0;
		}
		else if (hour < 10)
		{
			return 1;
		}
		else if (hour < 15)
		{
			return 2;
		}
		else if (hour < 19)
		{
			return 3;
		}
		else
		{
			return 4;
		}
	}
	else
	{
		return 4;
	}
}

//Convert a zone (1-1454) to a county
int DepartProbability::zoneToCounty(int zone)
{
	if (zone <= 190)
		return 75;
	else if (zone <= 346)
		return 81;
	else if (zone <= 714)
		return 85;
	else if (zone <= 1039)
		return 1;
	else if (zone <= 1210)
		return 13;
	else if (zone <= 1290)
		return 95;
	else if (zone <= 1317)
		return 55;
	else if (zone <= 1403)
		return 97;
	else if (zone <= 1454)
		return 41;
	return -1;
}

//Loads probabilities into memory
DepartProbability::DepartProbability()
{
	Timer t("Loading depart time probabilities");

	QuickParser q2(DEPART_PROBABILITY_FILE);

	for (int i = 0; i < 9; i++) //For each county
	{
		int county = q2.parseInt();	//Get county id
		for (int j = 0; j < 5; j++) //For each block of hours
		{
			int val = 0; //start probability at 0
			for (int k = 0; k < 4; k++)
			{
				val += q2.parseInt(); //increment for each probability seen
				if (k == 3) val = 100;	//ensure you end on 100%. prevents off-by-one errors (99, 101)
				probs[county].push_back(val);
			}
		}
		q2.parseNewLine();
	}
}

//Blank trip constuctor
Trip::Trip()
{
	group = NULL;
	if (!largeCalculations)
	{
		potentialSharing = new vector<int>();
		potentialSharing->reserve(20);

	}
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
	return shareable == 1;

}

//Sets a trip to be doable or not, recursively following any additions/subtractions
void Trip::setDoable(bool set)
{
	#pragma omp critical
	{
		if (set == true) //if we're making the trip doable
		{
			doable = true;
		}
		else//if we're making the trip not doable
		{
			if (!DoableTripModes[mode])
			{
				doable = false;
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
	milesDriven = 0;
	double rideShareProb = -97;
	double householdInteractionProb = -98;
	double totalScore = -99;
}

Household::Household()
{
	people.reserve(5);
	viable = true;
	jointMilesDriven = 0;
	indivMilesDriven = 0;
}