#ifndef GLOBALS_H
	#define GLOBALS_H

//#include "DataClasses.h"

#include <set>
#include <string>
#include <vector>

using namespace std;

const static double CLOSE_DISTANCE = 2.5;	//Two points must be within this to be considered closePoints. Make sure to update vector reserve() calls when changing this
const static int NUM_LOCATIONS = 1454;		//Number of locations in the simulation

const static string DISTANCE_FILE = "D:\\Farzad\\ridesharing\\sample data\\DistanceSkimsDatabaseAM.csv";
//const static string DISTANCE_FILE = "C:\\ITS\\DistanceSkimsDatabaseAM.csv";
const static int DISTANCE_FILE_SIZE = NUM_LOCATIONS * NUM_LOCATIONS;

const static string PERSON_FILE = "D:\\Farzad\\ridesharing\\sample data\\personFile.p2011s3a.2010.csv";
//const static string PERSON_FILE = "C:\\ITS\\personFile.p2011s3a.2010.csv";
//const static string PERSON_FILE = "C:\\ITS\\peopleTest.csv";
const static int PERSON_FILE_SIZE = 7053334;//7053334

const static string TOUR_FILE = "D:\\Farzad\\ridesharing\\sample data\\indivTourData_3.csv";
//const static string TOUR_FILE = "C:\\ITS\\indivTourData_3.csv";
//const static string TOUR_FILE = "C:\\ITS\\tourTest.csv";
const static int TOUR_FILE_SIZE = 8914778;//8914778

const static string TRIP_FILE = "D:\\Farzad\\ridesharing\\sample data\\indivTripData_3.csv";
//const static string TRIP_FILE = "C:\\ITS\\indivTripData_3.csv";
//const static string TRIP_FILE = "C:\\ITS\\tripTest.csv";
const static int TRIP_FILE_SIZE = 22811684;//22811684

static float TourDoableRequirement = 0.5;	//For some legs of a tour to be shared, at least this percent must be doable 

//Sharing algorithm variables:
static int MinPeople = 2;
static int MaxPeople = 5;

//Tour requirements

static int DoableTripModes[] = {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
//doable_trip_mode = [3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13] //must be shareable OR trip mode must be in this
//TourSharingProportion
//TourRandomFailChance
//Trip sharing requirements:  (ordered by computational complexity)

static int MinNumStops = 2; //Number of stops must be this or more

static int MaxIncome = 200000;						//Income must be below this

static float MinDistanceTraveled = 0.0;				//Distance between origin and dest. must be above this

static int TripModes[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };//1 represents that array index is shareable. Right now, indexes modes 1-7 (and not 0) are shareable
													
static unsigned int RandomFailChance = 0;					//%chance a trip will randomly not be shareable. This should be an integer from 0-100 

static set<string> TripPurposes = { "Home", "work_low", "work_med", "work_high", "work_very high", "university", "school_high", "school_grade", "atwork_business", "atwork_eat", "atwork_maint", \
"eatout", "escort_kids", "escort_no kids", "othdiscr", "othmaint", "shopping", "social" }; //Simply list acceptable purposes here


struct Tour; struct Trip; struct Person;

//Data allocation:
static Person* all_people;
static Tour* all_tours;
static Trip* all_trips;
static vector<short>* closePoints;
static bool close[NUM_LOCATIONS+1][NUM_LOCATIONS+1];
static vector<Trip*>* organized[24][NUM_LOCATIONS + 1];
static float* dist;



#define NOT_SHAREABLE 0
#define SHAREABLE 1
#define UNKNOWN -1
#define BEING_SHARED 2
#define SOLO 3
#define FOLLOWER 4
#define LEADER 5


#endif