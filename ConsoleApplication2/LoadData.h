#ifndef LOADDATA_H
#define LOADDATA_H

#include "stdafx.h"
#include <vector>
#include "DataClasses.h"
struct Trip;

void parseDistances();
void parseClosePoints();
void parseHouseholds();
void parsePeople();
void parseJointTours();
void parseTours();
void parseJointTrips();
vector<Trip*>& sortedTrips(int minute, int origin, int destination);
void parseTrips();

#endif