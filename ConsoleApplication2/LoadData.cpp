#include "stdafx.h"
#include "LoadData.h"
#include "Timer.h"
#include "QuickParser.h"
#include "QuickParser_inline.cpp"
#include "Globals.h"
#include "DataClasses.h"
#include "MiscFunctions.h"
#include "FastRand.h"

using namespace std;

double distanceBetween(int origin, int destination);

void parseDistances()
{
	QuickParser q(DISTANCE_FILE);
	int j = 0;
	while (j < DISTANCE_FILE_SIZE)
	{
		q.parseNewLine();
		q.parseComma();
		q.parseComma();
		dist[j++] = q.parseFloat();
	}
}

void parseClosePoints()
{

	Timer timeit("Parsing ClosePoints");

	parseDistances();

	//Check non-diagonal points where k > i. Graph is symmetric, so we don't need to check k < i
	for (int i = 1; i <= NUM_LOCATIONS; i++)
	{
		closePoints[i].push_back(i);
		for (int k = i + 1; k <= NUM_LOCATIONS; k++)
		{

			if (distanceBetween(i, k) < CLOSE_DISTANCE)
			{
				closePoints[i].push_back(k);
				closePoints[k].push_back(i);
				close[i][k] = 1;
				close[k][i] = 1;
			}
		}
	}
}

void parseHouseholds()
{
	QuickParser q(HOUSEHOLD_FILE)
	Timer timeit("Parsing households");

	for (int i = 0; i < HOUSEHOLD_FILE_SIZE; i++)
	{
		q.parseNewLine();

		int hhid = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		all_households[hhid].autos = q.parseInt();

		all_households[hhid].hhid = hhid;
	}
}

void parsePeople()
{
	QuickParser q(PERSON_FILE);

	Timer timeit("Parsing people");

	for (int i = 0; i < PERSON_FILE_SIZE; i++)
	{
		q.parseNewLine();

		int hhid  = q.parseInt();
		int perid = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		all_people[perid].income = q.parseInt();

		all_people[perid].id = perid;
		all_people[perid].hhid = hhid;

		all_households[hhid].people.push_back(&all_people[perid]);
	}
}

void parseJointTours()
{
	QuickParser q(JOINT_TOURS_FILE);

	Timer timeit("Parsing joint tours");

	for (int i = 0; i < JOINT_TOURS_FILE_SIZE; i++)
	{
		Tour& to = all_joint_tours[i];
		q.parseNewLine();
		to.hhid = q.parseInt();
		to.id = q.parseInt();

		all_households[to.hhid].tours[to.id] = &to;

	}
}

void parseTours()
{
	QuickParser q(TOUR_FILE);

	Timer timeit("Parsing tours");

	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		q.parseNewLine();

		int hhid = q.parseInt();
		int perid = q.parseInt();
		q.parseComma();
		q.parseComma();
		int tourid = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		all_tours[i].numStops = q.parseInt() + q.parseInt(); //Outbound stops + inbound stops


		all_tours[i].hhid = hhid;
		all_tours[i].id = tourid;
		all_people[perid].tours[tourid] = &all_tours[i];
	}
}

void parseJointTrips()
{
	QuickParser q(JOINT_TRIPS_FILE);
	Timer timeit("Paring joint trips");

	for (int i = 0; i < JOINT_TRIPS_FILE_SIZE; i++)
	{
		Trip& t = all_joint_trips[i];
		q.parseNewLine();

		t.id = i;
		int hhid = q.parseInt();
		t.tourid = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		t.purpose = q.parseString();
		t.origin = q.parseInt();
		q.parseComma();
		t.destination = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		t.mode = q.parseInt();

		all_households[hhid].tours[t.tourid]->trips.push_back(&t);

	}
}

void parseTrips()
{
	QuickParser q(TRIP_FILE);
	Timer timeit("Parsing trips");

	/*
	for (int i = 5; i < 24; i++)
	{
		for (int k = 1; k <= NUM_LOCATIONS; k++)
		{
			organized[i][k] = new vector<Trip*>[NUM_LOCATIONS + 1];
			organized[i][k]->reserve(120);
		}
	}*/

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& trip = all_trips[i];

		q.parseNewLine();

		q.parseComma();
		trip.perid = q.parseInt();
		q.parseComma();
		trip.tourid = q.parseInt();
		q.parseComma();
		q.parseComma();
		trip.purpose = q.parseString();
		q.parseComma();
		q.parseComma();
		trip.origin = q.parseInt();
		q.parseComma();
		trip.destination = q.parseInt();
		q.parseComma();
		q.parseComma();
		trip.hour = q.parseInt();
		trip.mode = q.parseInt();
		all_trips[i].id = i;

		trip.minute = (trip.hour * 60) + (departprobs->generate(trip.origin, trip.hour));

		if (DoableTripModes[trip.mode])
			trip.doable = true;

		if (ExecutionMode == 0)
		{
			if (trip.isShareable())
			{
				(*organized)[trip.hour][trip.origin][trip.destination].push_back(&all_trips[i]);
				shareable++;
			}
		}

		all_people[trip.perid].tours[trip.tourid]->trips.push_back(&all_trips[i]);
	}
}

