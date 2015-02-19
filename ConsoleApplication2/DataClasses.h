#ifndef DATACLASSES_H
#define DATACLASSES_H

#include "stdafx.h"
#include "Globals.h"

#include <map>
#include <vector>
#include <unordered_set>
#include <iostream>
using namespace std;

struct Trip
{
	int id;
	int perid;
	int tourid;
	int origin;
	int destination;
	int hour;
	int numPassengers;

	int mode;
	string purpose;

	vector<int> potentialSharing;
	//vector<int>* actualSharing;
	unordered_set<int>* actualSharing;

	int shareable; //1 = yes, 0 = no, -1 = unknown

	Trip();

	bool isShareable();
};



struct Tour
{
	int id;
	int hhid;
	int numStops;
	vector<Trip*> trips;

	int doableTripCount;

	Tour();
};

//Tourids are ordered 0-5, or 11 for an optional at-work lunch trip for some odd reason
struct Person
{	
	int id;
	int income;
	map<int, Tour*> tours;

	Person();
};

#endif