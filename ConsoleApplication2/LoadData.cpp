#include "stdafx.h"
#include "LoadData.h"
#include "Timer.h"
#include "QuickParser.h"
#include "QuickParser_inline.cpp"
#include "Globals.h"
#include "DataClasses.h"
#include "MiscFunctions.h"
#include "FastRand.h"

#include <omp.h>

using namespace std;


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
	int closec = 0;
	for (int i = 1; i <= NUM_LOCATIONS; i++)
	{
		for (int k = 1; k <= NUM_LOCATIONS; k++)
		{

			if (distanceBetween(i, k) < CLOSE_DISTANCE)
			{
				++closec;
				closePoints[i].push_back(k);
				close[i][k] = 1;
			}
		}
	}
	cout << "Each point is close to " << closec / NUM_LOCATIONS << " other points, on average." << endl;
}

 void parseHouseholds()
{
	QuickParser q(HOUSEHOLD_FILE);
	Timer timeit("Parsing households");

	for (int i = 0; i < HOUSEHOLD_FILE_SIZE; i++)
	{
		q.parseNewLine();
		int hhid = q.parseInt();	all_households[hhid].hhid = hhid;	Household& hh = all_households[hhid];
		/*
		q.parseComma();
		q.parseComma();
		q.parseComma();
		*/
		int t1 = q.parseInt();
		int t2 = q.parseInt();
		int t3 = q.parseInt();
		hh.income = q.parseInt();
		int t4 = q.parseInt();
		//q.parseComma();
		hh.type = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		hh.autos = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		hh.sizecat = q.parseInt();
		hh.familycat = q.parseInt();
		q.parseComma();
		hh.familychildren = q.parseInt();
		hh.familyworkers = q.parseInt();

		hh.viable = (hh.income < householdIncomeMax && 
			hh.autos <= householdVehiclesMax && 
			viableHouseholdTypes[hh.type] && 
			validSizeCat[hh.sizecat] &&
			validhfamily[hh.familycat] &&
			validhchildren[hh.familychildren] &&
			validhworker[hh.familyworkers]);

		householdIncome += hh.income;
		householdVehicles += hh.autos;
		householdType += hh.type;


	}
	householdIncome /= HOUSEHOLD_FILE_SIZE;
	householdVehicles /= HOUSEHOLD_FILE_SIZE;
	householdType /= HOUSEHOLD_FILE_SIZE;
}

void parsePeople()
{
	QuickParser q(PERSON_FILE);

	Timer timeit("Parsing people");

	for (int i = 0; i < PERSON_FILE_SIZE; i++)
	{
		q.parseNewLine();

		int hhid  = q.parseInt();
		int perid = q.parseInt(); Person& p = all_people[perid]; p.id = perid; p.hhid = hhid;
		p.age = q.parseInt();
		q.parseComma();
		p.esr = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		p.sex = q.parseInt();
		q.parseComma();
		q.parseComma();
		p.msp = q.parseInt();
		q.parseComma();
		p.income = q.parseInt();
		q.parseComma();
		p.pemploy = q.parseInt();
		q.parseComma();
		p.ptype = q.parseInt();

		if (p.age < maxAge && 
			validESR[p.esr] && 
			validSex[p.sex] && 
			validMSP[p.msp] && 
			validPTYPE[p.ptype] && 
			validPEmploy[p.pemploy] &&
			p.income < MaxIncome && p.income > MinIncome)
		{
			p.viable = true;
		}
		else
		{
			p.viable = false;
		}
		all_households[hhid].people.push_back(&p);
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


string purposes[] = { "work_low", "work_med", "work_high", "work_very high", "university", "school_high", "school_grade", "atwork_business", "atwork_eat", "atwork_maint", "eatout", "escort_kids",
"escort_no kids", "othdiscr", "othmaint", "shopping", "social" };

int purposeToInt(string purpose)
{
	for (int i = 0; i < 17; i++)
		if (purposes[i] == purpose)
			return i;
	return -1;
}

void parseJointTrips()
{
	QuickParser q(JOINT_TRIPS_FILE);
	Timer timeit("Parsing joint trips");

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
		t.purpose = purposeToInt(q.parseString());
		t.origin = q.parseInt();
		q.parseComma();
		t.destination = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		t.mode = q.parseInt();

		if (all_households[hhid].viable)
			all_households[hhid].tours[t.tourid]->trips.push_back(&t);

	}
}


vector<Trip*>* sortedTrips(int hour, int origin, int destination)
{
	return &organized[(hour * 2117025) + ((origin - 1) * 1455) + (destination - 1)];
}

void parseTrips()
{
	QuickParser q(TRIP_FILE);
	Timer timeit("Parsing trips");

	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& trip = all_trips[i];

		q.parseNewLine();

		int hhid = q.parseInt(); 
		trip.perid = q.parseInt();
		q.parseComma();
		trip.tourid = q.parseInt();
		q.parseComma();
		q.parseComma();
		trip.purpose = purposeToInt(q.parseString());
		q.parseComma();
		q.parseComma();
		trip.origin = q.parseInt();
		q.parseComma();
		trip.destination = q.parseInt();
		q.parseComma();
		q.parseComma();
		trip.hour = q.parseInt();
		trip.mode = q.parseInt();
		q.parseComma();
		trip.category = q.parseString();
		trip.id = i;

		trip.mandatory = (trip.category == "MANDATORY");

		trip.minute = (trip.hour * 60) + (departprobs->generate(trip.origin, trip.hour));

		if (DoableTripModes[trip.mode])
			trip.doable = true;

		if (ExecutionMode == 0 && trip.isShareable())
		{
			
			sortedTrips(trip.hour, trip.origin, trip.destination)->push_back(&all_trips[i]);
			//(*organized)[trip.minute][trip.origin][trip.destination].push_back(&all_trips[i]);
			shareable++;
		}

		all_people[trip.perid].tours[trip.tourid]->trips.push_back(&all_trips[i]);
	}
}

