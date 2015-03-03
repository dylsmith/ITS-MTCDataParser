#include "stdafx.h"
#include "Globals.h"
#include "MiscFunctions.h"
#include "QuickParser.h"
#include "Timer.h"

#include <iostream>
#include <map>
#include <omp.h>

using namespace std;


string lineModify(string input, string numPassengers, string mode)
{
	int commas = 0;
	int loc = -1;
	int start, end;

	while (commas != 2)
		if (input[++loc] == ',')
			commas++;
	start = loc;
	while (commas != 3)
		if (input[++loc] == ',')
			commas++;
	end = loc;

	input = input.substr(0, start + 1) + numPassengers + input.substr(end, input.size() - end);

	commas = 0;
	loc = -1;
	while (commas != 15)
		if (input[++loc] == ',')
			commas++;
	start = loc;
	while (commas != 16)
		if (input[++loc] == ',')
			commas++;
	end = loc;

	return input.substr(0, start + 1) + mode + input.substr(end, input.size() - end);
}

double distanceBetween(int origin, int destination)
{
	return dist[((origin - 1) * NUM_LOCATIONS) + (destination - 1)];
}

bool strictCompare(Trip& t1, Trip& t2)
{
	return (distanceBetween(t1.origin, t2.origin) < CLOSE_DISTANCE &&
		distanceBetween(t1.destination, t2.destination) < CLOSE_DISTANCE &&
		t1.perid != t2.perid);
}

void OMPInfo()
{
	cout << "Max threads: " << omp_get_max_threads() << endl;
	cout << "Max processors: " << omp_get_num_procs() << endl;
	cout << "Nested parallelism: " << omp_get_nested() << endl;
}

bool find(vector<short>& v, short val)
{
	return find(v.begin(), v.end(), val) != v.end();
}


int writeLoc = 0;
void write(double d)
{
	write(to_string(d) + "%");
}
void write(string s)
{

	int len = s.length();
	while (writeLoc > 0)
	{
		if (writeLoc >= len)
			cout << ' ' << '\b';
		writeLoc--;
		cout << '\b';
	}

	cout << s;
	writeLoc = len; 
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
		cout << '\'' << (int)p.first << "\': " << p.second << endl;
}


int lineCount(string filename)
{
	QuickParser q(filename);
	Timer c(" lines in " + filename);
	int lines = 0;
	for (int i = 0; i < q.length; i++)
		if (*(q.file + i) == '\n')
			lines++;
	cout << lines;
	return lines;
}


void largestTrips()
{
	QuickParser q(TRIP_FILE);
	int maxnum = 0;
	int num = 0;
	int lastperid = -1;
	int lasttourid = -1;
	for (int i = 0; i < TRIP_FILE_SIZE - 1; i++)
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
