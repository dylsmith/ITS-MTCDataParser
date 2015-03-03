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
	VGroup* group;

	bool doable;
	bool shared; //set to 0 if unshared by tour-level decisionss

	Trip();

	bool isShareable();
	void setDoable(bool set, bool recheckTour = true);
private:
	int shareable; //1 = yes, 0 = no, -1 = unknown. potential shareability
};



struct Tour
{
	int id;
	int hhid;
	int numStops;
	vector<Trip*> trips;
	bool shared;

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