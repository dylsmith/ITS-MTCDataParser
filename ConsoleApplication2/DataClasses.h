#ifndef DATACLASSES_H
	#define DATACLASSES_H

#include "stdafx.h"

#include <map>
#include <vector>
#include <iostream>
using namespace std;



struct Trip
{
	int id;
	int origin;
	int destination;
	int hour;
	vector<int> sharingList;

	Trip()
	{
		sharingList.reserve(4);
	};
};



struct Tour
{
	int id;
	int hhid;
	vector<Trip*> trips;

	Tour()
	{
		trips.reserve(3);
	};
};

//Tourids are ordered 0-5, or 11 for an optional at-work lunch trip for some odd reason
struct Person
{	
	int id;
	map<int, Tour*> tours;

	Person()
	{	
	};
};

#endif