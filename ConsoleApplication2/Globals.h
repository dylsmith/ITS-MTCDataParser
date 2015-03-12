#ifndef GLOBALS_H
	#define GLOBALS_H

//#include "DataClasses.h"
#include "stdafx.h"
#include <set>
#include <string>
#include <vector>
#include "MiscFunctions.h"

using namespace std;

extern int ExecutionMode; //0 = ridesharing, 1 = EV

struct Trip; struct DepartProbability;
typedef vector<Trip*> d0;
typedef vector<d0> d1;
typedef vector<d1> d2;
typedef vector<d2> d3;

extern string DATA_FILE;
extern string TRIP_DETAILS_FILE;
extern bool WriteTripDetails;
extern string TRIP_SHARING_FILE;
extern bool WriteTripSharing;
extern string SHARED_DETAILS_FILE;
extern string UNSHARED_DETAILS_FILE;

//EV algorithm variables:
extern double EVAverageRange;
extern int EVTripModes[];
extern string JOINT_TOURS_FILE;
extern int JOINT_TOURS_FILE_SIZE;
extern string JOINT_TRIPS_FILE;
extern int JOINT_TRIPS_FILE_SIZE;
extern string DEPART_PROBABILITY_FILE;
extern DepartProbability* departprobs;

//Sharing algorithm variables:
extern bool Maximize;
extern int MinPeople;	//If minizing, all groups must be at least this big
extern int MaxPeople;	//If maximizing, all groups cannot be larger than that
extern int MaxSharingTimeDifference; //How many minutes apart two shared trips can be

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

extern string HOUSEHOLD_FILE;
extern int HOUSEHOLD_FILE_SIZE;

extern string PERSON_FILE;
extern int PERSON_FILE_SIZE;

extern string TOUR_FILE;
extern int TOUR_FILE_SIZE;

extern string TRIP_FILE;
extern int TRIP_FILE_SIZE;


struct Tour; struct Trip; struct Person; struct Household;

//Data allocation:
extern Household* all_households;
extern Person* all_people;
extern Tour* all_tours;
extern Trip* all_trips;
extern Tour* all_joint_tours;
extern Trip* all_joint_trips;
extern vector<short>* closePoints;
extern bool close[1455][1455];
extern d3* organized;
extern float* dist;
extern int shareable;	//Trips that passed the initial checks
extern int potentialSharing;//Trips with at least one trip it could potentially share with	
extern int sharingBeforeTourLevel;//Trips that actually shared before tour-level checks
extern int sharingBeforeReshare;//Trips that actually shared before re=sharing
extern int actualSharing;//Trips that actually shared with at least one other trip
extern int unshared;//Trips unshared because of tour-level requirements
extern int groups;//Numer of sharing-groups
extern int solo;//Trips that could not be actually shared (but weren't unshared)
extern int orphaned;//Trips that were sharing with a trip that was unshared, and now are not sharing
extern double VMTReduction; //Vehicle miles saved


#endif