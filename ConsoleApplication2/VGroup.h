#ifndef VGROUP_H
#define VGROUP_H

#include "stdafx.h"
#include "DataClasses.h"
#include "Globals.h"
#include "MiscFunctions.h"

#include <list>

using namespace std;

struct Trip;

class VGroup
{
public:
	Trip* leader;
	list<Trip*> trips;

	//Constructs a group with t as leader
	VGroup(Trip& t);

	~VGroup();

	//If t2 can share with all elements in the group
	bool canAddTrip(Trip& t2);

	//Adds trip to group
	void addTrip(Trip& t2, bool recheckTour = true);

	//Removes trip from group
	void removeTrip(Trip& t1);

	//Removes trip from group, checks all other groups. Broken.
	void removeTripRecursive(Trip& t1);

private:
	void removeFromTrips(Trip& t);
};

#endif