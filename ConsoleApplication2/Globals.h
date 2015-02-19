#ifndef GLOBALS_H
	#define GLOBALS_H

//#include "DataClasses.h"
#include "stdafx.h"
#include <set>
#include <string>
#include <vector>
#include "MiscFunctions.h"

using namespace std;

/*
log file:
Max or Min
# of trips
# valid trpis
# of trips with nonzero potential sharing
# o frips with nonzero actual sharing
# of trips removed at tour level
# of trips orphaned at tour level
# of orphaned trips re-added //this will loop until 5%
//VMT reduction
*/

extern string DATA_FILE;
extern string TRIP_SHARING_FILE;

extern int shareable;	//Trips that passed the initial checks
extern int potentialSharing;//Trips with at least one trip it could potentially share with	
extern int actualSharing;//Trips that actually shared with at least one other trip
extern int unshared;//Trips unshared because of tour-level requirements
extern int solo;//Trips that could not be actually shared (but weren't unshared)
extern int orphaned;//Trips that were sharing with a trip that was unshared, and now are not sharing
extern double VMTReduction; //Vehicle miles saved

//Sharing algorithm variables:
extern bool Maximize;
extern int MinPeople;	//If minizing, all groups must be at least this big
extern int MaxPeople;	//If maximizing, all groups cannot be larger than that

//Tour requirements
extern float TourDoableRequirement;	//For some legs of a tour to be shared, at least this percent must be doable 
extern int DrivingModes[]; //Each ridesharing group must have one person whose mode is one of these
extern int DoableTripModes[];  //These modes do not require the trip to be shared for it to be doable


//Trip sharing requirements:  (ordered by computational complexity)
extern int MaxNumStops;	//Number of stops must be this or more
extern int MaxIncome; //Income must be below this
extern float MinDistanceTraveled;	//Distance between origin and dest. must be above this
extern int TripModes[]; //1 represents that array index is shareable. Right now, indexes modes 1-7 (and not 0) are shareable
extern unsigned int RandomFailChance; //%chance a trip will randomly not be shareable. This should be an integer from 0-100 
extern set<string> TripPurposes; //Simply list acceptable purposes here

//Count the number of files automatically
extern double CLOSE_DISTANCE;	//Two points must be within this to be considered closePoints. Make sure to update vector reserve() calls when changing this
extern int NUM_LOCATIONS;		//Number of locations in the simulation

extern string DISTANCE_FILE;
extern int DISTANCE_FILE_SIZE;

extern string PERSON_FILE;
extern int PERSON_FILE_SIZE;

extern string TOUR_FILE;
extern int TOUR_FILE_SIZE;

extern string TRIP_FILE;
extern int TRIP_FILE_SIZE;


struct Tour; struct Trip; struct Person;

//Data allocation:
extern Person* all_people;
extern Tour* all_tours;
extern Trip* all_trips;
extern vector<short>* closePoints;
extern bool close[1455][1455];
extern vector<Trip*>* organized[24][1455];
extern float* dist;

//Outdated
#define NOT_SHAREABLE 0
#define SHAREABLE 1
#define UNKNOWN -1
#define BEING_SHARED 2
#define SOLO 3
#define FOLLOWER 4
#define LEADER 5


#endif