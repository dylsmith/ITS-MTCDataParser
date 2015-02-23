#ifndef ITS_MISC
	#define ITS_MISC

#include "stdafx.h"

#include <iostream>
#include <map>

using namespace std;

//Generic remove function for STL classes that dont' support find() (linear search)
template<class T>
inline void remove(T& v, int trip)
{
	T::iterator it = v.begin();
	while (*it != trip)
		it++;
	v.erase(it);
}

void OMPInfo();

void averageSharedTrips();

bool find(vector<short>& v, short val);

void write(string s);

void commasperline();

void charsinfile();

int lineCount(string filename);

void largestTrips();

void largestTours();

#endif