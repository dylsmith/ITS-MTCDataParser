#ifndef INPUTVARS_H
#define INPUTVARS_H

#include "stdafx.h"
using namespace std;


extern int ExecutionMode; //0 = ridesharing, 1 = EV
extern bool largeCalculations; //If true, will determine trip sharing on-the-fly rather than saving sharing sets in memory


extern double rideShareWeight;
extern double householdInteractionWeight;

extern double mandatoryTripWeight;
extern double nonMandatoryTripWeight;
extern double PercentTripsToShare;

#endif
