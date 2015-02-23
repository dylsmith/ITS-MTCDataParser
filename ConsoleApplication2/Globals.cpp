
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

string DATA_FILE = "C:\\ITS\\Output.txt";
string TRIP_SHARING_FILE = "C:\\ITS\\TripSharing.txt";

int shareable = 0;	//Trips that passed the initial checks
int potentialSharing = 0;//Trips with at least one trip it could potentially share with	
int sharingBeforeTourLevel = 0;//Trips that actually shared before tour-level checks
int sharingBeforeReshare = 0;//Trips that actually shared before re=sharing
int actualSharing = 0;//Trips that actually shared with at least one other trip
int unshared = 0;//Trips unshared because of tour-level requirements
int solo = 0;//Trips that could not be actually shared (but weren't unshared)
int orphaned = 0;//Trips that were sharing with a trip that was unshared, and now are not sharing
double VMTReduction = 0; //Vehicle miles saved

//Sharing algorithm variables:
bool Maximize = true;
int MinPeople = 2;	//If minizing, all groups must be at least this big
int MaxPeople = 5;	//If maximizing, all groups cannot be larger than that

//Tour requirements
float TourDoableRequirement = 0.5;	//For some legs of a tour to be shared, at least this percent must be doable 
int DrivingModes[] = { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //Each ridesharing group must have one person whose mode is one of these
int DoableTripModes[] = { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };  //These modes do not require the trip to be shared for it to be doable


//Trip sharing requirements:  (ordered by computational complexity)
int MaxNumStops = 6;	//Number of stops must be this or more
int MaxIncome = 200000; //Income must be below this
float MinDistanceTraveled = 1.0;	//Distance between origin and dest. must be above this
int TripModes[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; //1 represents that array index is shareable. Right now, indexes modes 1-7 (and not 0) are shareable
unsigned int RandomFailChance = 0; //%chance a trip will randomly not be shareable. This should be an integer from 0-100 
set<string> TripPurposes = { "Home", "work_low", "work_med", "work_high", "work_very high", "university", "school_high", "school_grade", "atwork_business", "atwork_eat", "atwork_maint", \
"eatout", "escort_kids", "escort_no kids", "othdiscr", "othmaint", "shopping", "social" }; //Simply list acceptable purposes here

//Count the number of files automatically
double CLOSE_DISTANCE = 1.0;	//Two points must be within this to be considered closePoints. Make sure to update vector reserve() calls when changing this
int NUM_LOCATIONS = 1454;		//Number of locations in the simulation

//string DISTANCE_FILE = "D:\\Farzad\\ridesharing\\sample data\\DistanceSkimsDatabaseAM.csv";
string DISTANCE_FILE = "C:\\ITS\\DistanceSkimsDatabaseAM.csv";
int DISTANCE_FILE_SIZE = NUM_LOCATIONS * NUM_LOCATIONS;

//string PERSON_FILE = "D:\\Farzad\\ridesharing\\sample data\\personFile.p2011s3a.2010.csv";
string PERSON_FILE = "C:\\ITS\\personFile.p2011s3a.2010.csv";
int PERSON_FILE_SIZE = lineCount(PERSON_FILE) - 1;//7053334

//string TOUR_FILE = "D:\\Farzad\\ridesharing\\sample data\\indivTourData_3.csv";
string TOUR_FILE = "C:\\ITS\\indivTourData_3.csv";
int TOUR_FILE_SIZE = lineCount(TOUR_FILE) - 1;//8914778

//string TRIP_FILE = "D:\\Farzad\\ridesharing\\sample data\\indivTripData_3.csv";
string TRIP_FILE = "C:\\ITS\\indivTripData_3.csv";
int TRIP_FILE_SIZE = lineCount(TRIP_FILE) - 1;//22811684


struct Tour; struct Trip; struct Person;

//Data allocation:
Person* all_people;
Tour* all_tours;
Trip* all_trips;
vector<short>* closePoints;
bool close[1455][1455];
vector<Trip*>* organized[24][1455];
float* dist;

//Outdated
#define NOT_SHAREABLE 0
#define SHAREABLE 1
#define UNKNOWN -1
#define BEING_SHARED 2
#define SOLO 3
#define FOLLOWER 4
#define LEADER 5

