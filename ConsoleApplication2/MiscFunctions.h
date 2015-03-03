#ifndef ITS_MISC
	#define ITS_MISC

#include "stdafx.h"
#include "DataClasses.h"

#include <iostream>
#include <map>
#include <list>

using namespace std;

string lineModify(string input, string numPassengers, string mode);

double distanceBetween(int origin, int destination);

//Compares two trips, checking every requirement
struct Trip;
bool strictCompare(Trip& t1, Trip& t2);

//Generic remove function for STL classes that dont' support find() (linear search)
//template<class T>
inline void remove(list<int>& v, int trip)
{
	list<int>::iterator it = v.begin();
	while (*it != trip)
		it++;
	v.erase(it);
}

void OMPInfo();

void averageSharedTrips();

bool find(vector<short>& v, short val);

void write(double d);
void write(string s);

void commasperline();

void charsinfile();

int lineCount(string filename);

void largestTrips();

void largestTours();

#endif