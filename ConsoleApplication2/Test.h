#include "stdafx.h"
#include "DataClasses.h"
#include "Globals.h"
#include "VGroup.h"
#include "Timer.h"
#include <iostream>
#include <set>
#include <vector>

using namespace std;
#define MinutesInDay 24 * 60


inline set<Trip*>* sortedTrips(set<Trip*>* mOrganized, int hour, int origin, int destination)
{
	return &mOrganized[(hour * 2117025) + ((origin - 1) * 1455) + (destination - 1)];
}

inline bool compareTripsTest(Trip& trip1, Trip& trip2)
{
	return (trip1.perid != trip2.perid &&
		abs(trip1.minute - trip2.minute) <= MaxSharingTimeDifference);// &&
	distanceBetween(trip1.origin, trip2.origin) < CLOSE_DISTANCE &&
		distanceBetween(trip1.destination, trip2.destination) < CLOSE_DISTANCE;
}

void testShare()
{
	Timer ti("test share");
	vector<Trip*> tripsAtMinute[MinutesInDay];
	for (int minute = 0; minute < MinutesInDay; minute++)
	{
		//tripsAtMinute[minute].reserve(TRIP_FILE_SIZE / MinutesInDay);

	}

	set<Trip*>* mOrganized = new set<Trip*>[1455 * 1455 * 24]();
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& t = all_trips[i];
		if (t.isShareable())
		{
			tripsAtMinute[t.minute].push_back(&t);
			potentialSharing++;
		}
	}

	int groupSize = 5;
	set<Trip*> active;
	for (int minute = 0; minute < MinutesInDay; minute++)
	{
		cout << minute << endl;
		for (Trip* t : tripsAtMinute[minute])
		{
			sortedTrips(mOrganized, t->hour, t->origin, t->destination)->insert(t);
			active.insert(t);
		}

		if (active.size() < 1)
			continue;
		for (set<Trip*>::iterator it = active.begin(); it != active.end(); it++)
		//for (Trip* t1 : active)
		{
			Trip* t1 = *it;
			set<Trip*> maybeShared;

			//for each trip in each minute, starting from me and working outwards

			bool breaking = false;
			//for each sorted trip by sorted order
			for (int hour = t1->hour - 1; hour <= t1->hour + 1; hour++)
			{
				if (hour < 0 || hour >= 24)
					continue;
				for (int closeOrigin : closePoints[t1->origin])
				{
					for (int closeDestination : closePoints[t1->destination])
					{
						if (sortedTrips(mOrganized, hour, closeOrigin, closeDestination)->size() < 0)
							continue;
						set<Trip*>& trips = *sortedTrips(mOrganized, hour, closeOrigin, closeDestination);

						//for (set<Trip*>::iterator it = trips.begin(); it != trips.end(); it++)
						for (Trip* t2 : *sortedTrips(mOrganized, hour, closeOrigin, closeDestination))
						{
							//Trip* t2 = *it;
							if (t1->perid != t2->perid && abs(t1->minute - t2->minute) <= MaxSharingTimeDifference)
							{
								maybeShared.insert(t2);
								if (maybeShared.size() - 1 == groupSize)
								{
									t1->group = new VGroup(*t1);
									for (Trip* t3 : maybeShared)
									{
										t1->group->trips.push_back(t3);
										t3->group = t1->group;
									}
									maybeShared.insert(t1);

									//send off trips!

									for (Trip* t3 : maybeShared)
									{
										/*
										if (t3 == *it)
											it = trips.erase(it);
										else
											*/
										trips.erase(t3);
										if (*it == t3)
											it = active.erase(it);
										else
											active.erase(t3);
									}
									breaking = true;
								}
							}
							if (breaking) break;
						}
						if (breaking) break;
					}
					if (breaking) break;
				}
				if (breaking) break;
			}
		}
	}
}