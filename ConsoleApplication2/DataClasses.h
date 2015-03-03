#ifndef DATACLASSES_H
#define DATACLASSES_H

#include "stdafx.h"
#include "Globals.h"
#include "VGroup.h"

#include <map>
#include <vector>
#include <list>
#include <iostream>
using namespace std;


class VGroup;
struct Trip
{
	//vars from file:
	int id;
	int perid;
	int tourid;
	int origin;
	int destination;
	int hour;
	int numPassengers;
	int mode;
	string purpose;

	//generated vars:
	vector<int> potentialSharing;
	VGroup* group;
	bool doable;
	bool shared; 

	Trip();
	bool isShareable();
	void setDoable(bool set, bool recheckTour = true);

private:
	int shareable; //1 = yes, 0 = no, -1 = unknown. potential shareability
};

struct Tour
{
	//vars from file:
	int id;
	int hhid;
	int numStops;

	//generated vars:
	vector<Trip*> trips;
	bool shared;
	
	Tour();
};

struct Person
{	
	//vars from file:
	int id;
	int income;

	//generated vars:
	map<int, Tour*> tours;

	Person();
};

#endif