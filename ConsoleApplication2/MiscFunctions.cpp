

#include "stdafx.h"
#include "Globals.h"
#include "MiscFunctions.h"
#include "QuickParser.h"
#include "Timer.h"

#include <iostream>
#include <map>

using namespace std;

bool find(vector<short>& v, short val)
{
	return find(v.begin(), v.end(), val) != v.end();
}


int writeLoc = 0;
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
	Timer c("Counting lines in " + filename);
	int lines = 0;
	for (int i = 0; i < q.length; i++)
		if (*(q.file + i) == '\n')
			lines++;
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
