#ifndef DATACLASSES_H
#define DATACLASSES_H

#include "stdafx.h"
#include "Globals.h"
#include "VGroup.h"
#include "QuickParser.h"
#include "FastRand.h"

#include <map>
#include <vector>
#include <list>
#include <iostream>
using namespace std;

struct DepartProbability
{
	map<int, vector<int> > probs; 

	//Generate a random minute offset, based on observed departure quartiles
	int generate(int zone, int hour);

	//Convert an hour to a probability block (chunks of hours)
	int hourToBlock(int hour);

	//Convert a zone (1-1454) to a county
	int zoneToCounty(int zone);
	
	DepartProbability();
};

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
	int minute;
	int mode;
	string purpose;
	string category;

	//generated vars:
	vector<int>* potentialSharing;
	VGroup* group;
	bool doable;
	bool shared; 
	bool mandatory;

	Trip();
	bool isShareable();
	void setDoable(bool set, bool recheckTour);

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
	int hhid;

	double milesDriven;

	//generated vars:
	map<int, Tour*> tours;

	double rideShareProb;
	double householdInteractionProb;
	double totalScore;

	Person();
};

struct Household
{
	int hhid;
	int autos;
	double jointMilesDriven;
	double indivMilesDriven;
	vector<Person*> people;
	map<int, Tour*> tours;

	bool viable;

	Household();
};

#endif