#include "stdafx.h"
#include "DataClasses.h"
#include "FastRand.h"
#include "Globals.h"
#include "MiscFunctions.h"

#include <map>
#include <vector>
#include <unordered_set>
#include <iostream>
using namespace std;

double distanceBetween(int origin, int destination)
{
	return dist[((origin - 1) * NUM_LOCATIONS) + (destination - 1)];
}

Trip::Trip()
{
	potentialSharing.reserve(4);
	shareable = UNKNOWN;
}



Tour::Tour()
{
	trips.reserve(5);
	doableTripCount = 0;
}

Person::Person()
{
}