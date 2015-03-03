#include "stdafx.h"
#include "VGroup.h"
#include "DataClasses.h"

using namespace std;

bool VGroup::canAddTrip(Trip& t2)
{
	if ((t2.group && t2.group->trips.size() > 1) || !t2.shared)
		return false;
	
	int numPassengers = 0;
	for (Trip* t1 : trips)
	{
		if (!strictCompare(*t1, t2))
		{
			return false;
		}
		numPassengers += t1->numPassengers;
	}
	numPassengers += t2.numPassengers;

	if (Maximize)
		return(numPassengers <= MaxPeople);
	else
		return(numPassengers >= MinPeople);

}

void VGroup::addTrip(Trip& t2, bool recheckTour)
{
	trips.push_back(&t2);
	leader->setDoable(true, recheckTour);
	t2.setDoable(true, recheckTour);
	if (t2.group) delete t2.group;
	t2.group = this;

}

void VGroup::removeTrip(Trip& t1)
{
	if (t1.shared)
	{
		t1.shared = false;
		t1.setDoable(false, false);
		int size = trips.size();
		//if (size > 1)
			unshared++;

		if (size == 2)
		{
			Trip* t2;
			if (*trips.begin() == &t1)
				t2 = *trips.rbegin();
			else
				t2 = *trips.begin();

			delete t1.group;
			t1.group = new VGroup(t1);
			
			//removeFromTrips(*t2);
			t2->group = NULL;
			//t2->group = new VGroup(*t2);
			t2->setDoable(false, false);
		}
		else if (size > 2)
		{
			removeFromTrips(t1);
			VGroup* group = t1.group;
			t1.group = new VGroup(t1);
			bool foundNewDriver = false;
			for (Trip* t2 : group->trips)
			{
				if (DrivingModes[t2->mode])
				{
					foundNewDriver = true;
					group->leader = t2;
					break;
				}
			}
			if (!foundNewDriver)
			{
				for (Trip* t2 : group->trips)
				{
					t2->setDoable(false, false);
					//t2->group = NULL;
					t2->group = new VGroup(*t2);
				}
				delete group;
			}
		}
	}
}

void VGroup::removeTripRecursive(Trip& t1)
{
	if (t1.shared)
	{
		t1.shared = false;
		t1.setDoable(false);
		int size = trips.size();
		if (size == 2)
		{
			Trip* t2;
			if (*trips.begin() == &t1)
				t2 = *trips.rbegin();
			else
				t2 = *trips.begin();

			removeFromTrips(*t2);
			t2->group = NULL;
			t2->setDoable(false);

		}
		else if (size > 2)
		{
			removeFromTrips(t1);
			VGroup* group = t1.group;
			t1.group = new VGroup(t1);
			bool foundNewDriver = false;
			for (Trip* t2 : group->trips)
			{
				if (DrivingModes[t2->mode])
				{
					foundNewDriver = true;
					group->leader = t2;
					break;
				}
			}
			if (!foundNewDriver)
			{
				for (Trip* t2 : group->trips)
				{
					removeTrip(*t2);
				}
			}
		}
	}
}

void VGroup::removeFromTrips(Trip& t)
{
	list<Trip*>::iterator it = trips.begin();
	while (*it != &t)
		it++;
	trips.erase(it);
}