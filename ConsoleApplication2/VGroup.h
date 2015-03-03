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

	VGroup(Trip& t)
	{
		leader = &t;
		trips.push_back(&t);
	}

	bool canAddTrip(Trip& t2);
	void addTrip(Trip& t2, bool recheckTour = true);
	void removeTrip(Trip& t1);
	void removeTripRecursive(Trip& t1);

private:
	void removeFromTrips(Trip& t);
};

#endif