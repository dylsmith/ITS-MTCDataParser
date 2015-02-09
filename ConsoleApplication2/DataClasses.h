#ifndef DATACLASSES_H
#define DATACLASSES_H

#include "stdafx.h"
#include "FastRand.h"
#include "Globals.h"
#include "MiscFunctions.h"

#include <map>
#include <vector>
#include <unordered_set>
#include <iostream>
using namespace std;



struct Trip
{
	int id;
	int perid;
	int origin;
	int destination;
	int hour;
	int numPassengers;

	int income;
	int mode;
	string purpose;

	vector<int> sharingList;
	vector<int> actualSharing;

	int shareable; //1 = yes, 0 = no, -1 = unknown

	Trip()
	{
		sharingList.reserve(4);
		shareable = -1;
	}

	bool isShareable()
	{
		if (shareable == -1)
			shareable = (
				distanceBetween(origin, destination) > MinDistanceTraveled &&	//Distance check
				income < MaxIncome &&
				TripModes[mode] == 1 &&											//Mode is valid check  (array index of the mode must be 1)
				((fastrand() % 100) + 1) > RandomFailChance &&					//Random chance (random int must be greater than fail chance)
				TripPurposes.find(purpose) != TripPurposes.end()				//Purpose check (purpose must be in the set of allowed purposes)
			);
		return shareable;
		
	}
};



struct Tour
{
	int id;
	int hhid;
	vector<Trip*> trips;

	Tour()
	{
		trips.reserve(5);
	};
};

//Tourids are ordered 0-5, or 11 for an optional at-work lunch trip for some odd reason
struct Person
{	
	int id;
	int income;
	map<int, Tour*> tours;

	Person()
	{	
	};
};

#endif