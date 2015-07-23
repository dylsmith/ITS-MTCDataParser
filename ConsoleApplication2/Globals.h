#ifndef GLOBALS_H
	#define GLOBALS_H

#include "stdafx.h"
#include <set>
#include <string>
#include <vector>
#include <mutex>
#include "MiscFunctions.h"
#include <array>	

using namespace std;

extern bool parallel;
extern int ExecutionMode; //0 = ridesharing, 1 = EV
extern bool largeCalculations; //If true, will determine trip sharing on-the-fly rather than saving sharing sets in memory

extern double rideShareWeight;
extern double householdInteractionWeight;

extern double mandatoryTripWeight;
extern double nonMandatoryTripWeight;
extern double PercentTripsToShare;


struct Trip; struct DepartProbability;
typedef vector<Trip*> d0;
typedef vector<d0> d1;
typedef vector<d1> d2;
typedef vector<d2> d3;

extern string ALL_TRIP_DETAILS_FILE;
extern string DATA_FILE;
extern bool WriteTripDetails;
extern bool WriteTripSharing;
extern string TRIP_DETAILS_FILE;
extern string SHARED_DETAILS_FILE;
extern string UNSHARED_DETAILS_FILE;
extern string TRIP_SHARING_FILE;

//EV algorithm variables:
extern int viableHouseholds;
extern double EVAverageRange;
extern array<int, 19> EVTripModes;
extern string JOINT_TOURS_FILE;
extern int JOINT_TOURS_FILE_SIZE;
extern string JOINT_TRIPS_FILE;
extern int JOINT_TRIPS_FILE_SIZE;
extern string DEPART_PROBABILITY_FILE;
extern DepartProbability* departprobs;
extern string SHARED_PERSON_FILE;
extern string UNSHARED_PERSON_FILE;
extern string HOUSEHOLD_EV_FILE;
extern bool WriteInducedDemand;

//Sharing algorithm variables:
extern bool Maximize;
extern int MinPeople;	//If minizing, all groups must be at least this big
extern int MaxPeople;	//If maximizing, all groups cannot be larger than that
extern int MaxSharingTimeDifference; //How many minutes apart two shared trips can be

//Tour requirements
extern float TourDoableRequirement;	//For some legs of a tour to be shared, at least this percent must be doable 
extern array<int, 19> DrivingModes; //Each ridesharing group must have one person whose mode is one of these
extern array<int, 19> DoableTripModes;  //These modes do not require the trip to be shared for it to be doable
extern array<int, 17> TourPurposes;

//Trip sharing requirements:  (ordered by computational complexity)
extern int MaxNumStops;	//Number of stops must be this or more
extern int MaxIncome; //Income must be below this
extern int MinIncome;
extern float MinDistanceTraveled;	//Distance between origin and dest. must be above this
extern float MaxDistanceTraveled;
extern array<int, 19> TripModes; //1 represents that array index is shareable. Right now, indexes modes 1-7 (and not 0) are shareable
extern unsigned int RandomFailChance; //%chance a trip will randomly not be shareable. This should be an integer from 0-100 
extern array<int, 18> TripPurposes; //Simply list acceptable purposes here

extern int minOriginZone;
extern int maxOriginZone;
extern int minDestinationZone;
extern int maxDestinationZone;



//Household
extern double householdIncomeMax;
extern array<int, 8> viableHouseholdTypes;
extern int householdVehiclesMax;
extern array<int, 5> validSizeCat;
extern array<int, 3> validhfamily;
extern array<int, 2> validhchildren;
extern array<int, 3> validhworker;

//Person restrictions
extern int maxAge;
//                0  1  2  3  4  5  6
extern array<int, 7> validESR;
extern array<int, 3> validSex; //1 = male, 2 = female
extern array<int, 7> validMSP;
extern array<int, 9> validPTYPE;
extern array<int, 5> validPEmploy;

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
//extern vector<Person*> all_people;
extern Person* all_people;
extern Tour* all_tours;
extern Trip* all_trips;
extern Tour* all_joint_tours;
extern Trip* all_joint_trips;
extern vector<short>* closePoints;
extern bool close[1455][1455];
extern vector<Trip*>* organized;
//extern d3* organized;
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

extern double householdIncome; //Average household income
extern double householdType; //?? TODO: What is this
extern double householdVehicles; //Average household vehicles

extern bool sortPotentialSharing; 
extern double sharingRequirementStep;
extern double sharingRequirement;

extern int g_seed;
extern wstring path;
extern int debug;

#endif