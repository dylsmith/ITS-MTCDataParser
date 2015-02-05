#include "stdafx.h"

#include "ClosePoints.h"
#include "DataClasses.h"
#include "Distances.h"
#include "Globals.h"
#include "QuickParser.h"
#include "QuickParser_inline.cpp"
#include "Timer.h"

#include <iostream>
#include "omp.h"
using namespace std;


int lineCount(string filename)
{
	QuickParser q(filename);
	Timer c("Counting lines in " + filename);
	int lines = 0;
	for (int i = 0; i < q.length; i++)
		if (*(q.file + i) == '\n')
			lines++;
	return lines;
}

void parsePeople()
{
	QuickParser q(PERSON_FILE);

	Timer timeit("Parsing people");

	#pragma omp parallel for
	for (int i = 0; i < PERSON_FILE_SIZE; i++)
	{
		q.parseNewLine();
		q.parseComma();
		int perid = q.parseInt();
		all_people[perid].id = perid;
	}
}

void parseTours()
{
	QuickParser q(TOUR_FILE);

	Timer timeit("Parsing tours");

	#pragma omp parallel for
	//for (int i = 0; i < 10; i++)
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		q.parseNewLine();
		
		int hhid = q.parseInt();
		int perid = q.parseInt();
		q.parseComma();
		q.parseComma();
		int tourid = q.parseInt();


		all_tours[i].hhid = hhid;
		all_tours[i].id = tourid;
		all_people[perid].tours[tourid] = &all_tours[i];
	}
}

void parseTrips()
{
	QuickParser q(TRIP_FILE);

	Timer timeit("Parsing trips");


	for (int i = 0; i < 24; i++)
	{
		organized[i] = new vector<Trip*>[NUM_LOCATIONS + 1];
		for (int k = 0; k < NUM_LOCATIONS+1; k++)
		{
			organized[i][k].reserve(700);
		}
	}


	//#pragma omp parallel for
	for (int i = 0; i < TRIP_FILE_SIZE; i++)
	{
		Trip& trip = all_trips[i];

		q.parseNewLine();

		q.parseComma();
		int perid = q.parseInt();
		q.parseComma();
		int tourid = q.parseInt();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		q.parseComma();
		trip.origin = q.parseInt();
		q.parseComma();
		trip.destination = q.parseInt();
		q.parseComma();
		q.parseComma();
		trip.hour = q.parseInt();
		all_trips[i].id = i;



		organized[trip.hour][trip.origin].push_back(&all_trips[i]);
		//all_people[perid].tours[tourid]->trips.push_back(&all_trips[i]);
	}
}

void largestTrips()
{
	QuickParser q(TRIP_FILE);
	int maxnum = 0;
	int num = 0;
	int lastperid = -1;
	int lasttourid = -1;
	for (int i = 0; i < TRIP_FILE_SIZE-1; i++)
	{
		q.parseNewLine();
		q.parseComma();
		int perid = q.parseInt();
		q.parseComma();
		int tourid = q.parseInt();
		if (perid == lastperid && tourid == lasttourid)
		{
			num++;
		}
		else
		{
			if (num > maxnum)
				maxnum = num;
			num = 1;
			lastperid = perid;
			lasttourid = tourid;
		}
	}
	cout << "Max number of trips in a single tour is " << maxnum << endl;	//17
}

void largestTours()
{
	QuickParser q(TOUR_FILE);
	int maxnum = 0;
	int num = 0;
	int lastperid = -1;
	for (int i = 0; i < TOUR_FILE_SIZE; i++)
	{
		q.parseNewLine();
		q.parseComma();
		int perid = q.parseInt();
		if (perid == lastperid)
		{
			num++;
		}
		else
		{
			if (num > maxnum)
				maxnum = num;
			num = 1;
			lastperid = perid;
		}
	}
	cout << "Max number of tours from a single person is " << maxnum << endl;//8
}

void reserveSpace()
{
	Timer t("Reserving space for all objects");
	all_people = new Person[PERSON_FILE_SIZE + 1];
	all_tours = new Tour[TOUR_FILE_SIZE];
	all_trips = new Trip[TRIP_FILE_SIZE];
	closePoints = new vector<short>[NUM_LOCATIONS + 1];
}

#include<map>
void charsinfile()
{
	QuickParser q(TOUR_FILE);
	map<char, int> vals;
	for (int i = 0; i < q.length; i++)
	{
		auto& it = vals.find(*(q.file + i));
		if (it == vals.end())
			vals[*(q.file + i)] = 1;
		else
			(*it).second++;
		if (i % 1000000 == 0) cout << i << endl;
	}

	for (auto& p : vals)
		cout << '\'' <<  (int)p.first << "\': " << p.second << endl;
}

void commasperline()
{
	QuickParser q(TOUR_FILE);
	int commas = 0, mincommas = 999, curcommas = 0, maxcommas = 0;
	int newlines = 0;
	for (int i = 0; i < q.length; i++)
	{
		if (*(q.file + i) == '\n' || *q.file == '\r')
		{
			if (curcommas < mincommas)
				mincommas = curcommas;
			else if (curcommas > maxcommas)
				maxcommas = curcommas;

			if (curcommas < 16)
				cout << curcommas << " commas on line " << newlines << endl;
			curcommas = 0;
			newlines++;
		}
		else if (*(q.file + i) == ',')
		{
			curcommas++;
		}
	}
	cout << "Min commas: " << mincommas << endl << "Max commas: " << maxcommas << endl << "newlines: " << newlines << endl;
}

inline bool find(vector<short>& v, short val)
{
	return find(v.begin(), v.end(), val) != v.end();
}

bool compareTrips(Trip& trip1, Trip& trip2)
{
	return (
		trip1.id != trip2.id &&
		trip1.hour == trip2.hour &&
		find(closePoints[trip1.origin], trip2.origin) &&
		find(closePoints[trip2.destination], trip2.destination)
		);
}

void analyzeTrips()
{
	Timer ct("Comparing trips");

	#pragma omp parallel for

	long int sharedtrips = 0;

	long long int totalloop = 0;
	#pragma omp parallel for
	for (int hour = 0; hour < 24; hour++)
	{
		for (int origin = 1; origin <= NUM_LOCATIONS; origin++)
		{
			for (Trip* trip1 : organized[hour][origin])
			{
				for (int closePoint : closePoints[origin])
				{
					for (Trip* trip2 : organized[hour][closePoint])
					{
						totalloop++;
						if (totalloop % 150000000 == 0) cout << totalloop << endl;
						if (compareTrips(*trip1, *trip2))
						{
							trip1->sharingList.push_back(trip2->id);
							trip2->sharingList.push_back(trip1->id);
							sharedtrips++;
						}
					}
				}
			}
		}
	}
			
	cout << ((long double)sharedtrips / TRIP_FILE_SIZE) / 100 << totalloop << " % of trips were shared." << endl;
}

void generateClosePoints(Distances &dist)
{

	Timer timeit("Generating ClosePoints");

	int p = 0;
	//Check non-diagonal points where k > i. Graph is symmetric, so we don't need to check k < i
	for (int i = 1; i <= NUM_LOCATIONS; i++)
	{
		for (int k = i + 1; k <= NUM_LOCATIONS; k++)
		{
			if (dist(i, k) < CLOSE_DISTANCE)
			{
				closePoints[i].push_back(k);
				closePoints[k].push_back(i);
			}
		}
	}

	//Check diagonal points (k = i)
	for (int i = 1; i <= NUM_LOCATIONS; i++)
	{
		if (dist(i, i) < CLOSE_DISTANCE)
		{
			closePoints[i].push_back(i);
		}
	}
}

void timerWrapper()
{
	Timer total("Total");
	reserveSpace();

	Distances dist(DISTANCE_FILE);
	generateClosePoints(dist);

	parsePeople();
	parseTours();
	parseTrips();
	analyzeTrips();
}

int _tmain(int argc, _TCHAR* argv[])
{
	timerWrapper();
	
	pause();
	return 0;
}


