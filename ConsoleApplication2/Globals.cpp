
#include "stdafx.h"
#include <set>
#include <string>
#include <vector>
#include "MiscFunctions.h"
#include <array>
struct DepartProbability;
using namespace std;

int ExecutionMode = 0; // 0 = ridesharing, 1 = EV
bool largeCalculations = true; //If true, will determine trip sharing on-the-fly rather than saving sharing sets in memory (This is good for scenarios with closedistance greater than 2.5)

//Induced Person Demand
double rideShareWeight = 1.0; // total inividual weighting factor 
double householdInteractionWeight = 1.0; // total hh weighting factor
double mandatoryTripWeight = 2.0;
double nonMandatoryTripWeight = 1.0;
double TripsToShare = 0.1; // 10% of trips are ridesharable (fixing them)
double sharingRequirementStep = 3.0; // 3% decremental step

//Output settings
string PERSON_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\PersonOutput.csv"; //Person file output
string ALL_TRIP_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\AllTripDetails.csv"; //All trips printed out here, with modes changed to '5' if in a group
string DATA_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\DataPoints.txt";//Important data points written here
string TRIP_SHARING_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\TripSharing.txt";//Each tripid and its actual sharing list will be written to this file
bool WriteTripSharing = false;
string TRIP_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\TripDetails.csv";	//Shared trips will be merged (and mode changed to 5), unshared trips left intact, and written to this
string SHARED_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\SharedTripDetails.csv"; //Split versions of above
string UNSHARED_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\UnsharedTripDetails.csv";
string SHARED_PERSON_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\SharedPersonDetails.csv"; // person file for induced demand 
bool WritePersonDetails = false;
string UNSHARED_PERSON_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\new_max\\UnsharedPersonDetails.csv"; // person file for induced demand
string HOUSEHOLD_EV_FILE = "D:\\MTC_BASE\\PostProcess\\HouseholdEVData_100.csv";
bool WriteTripDetails = false;


//EV algorithm variables:
int viableHouseholds = 0;
double EVAverageRange = 100; //100 -> 92.9% //EV range
int EVTripModes[] = { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //Identical to drivingmodes right now
string JOINT_TOURS_FILE = "D:\\MTC_BASE\\main\\jointTourData_4.csv";
int JOINT_TOURS_FILE_SIZE = lineCount(JOINT_TOURS_FILE) - 1;
string JOINT_TRIPS_FILE = "D:\\MTC_BASE\\main\\jointTripData_4.csv";
int JOINT_TRIPS_FILE_SIZE = lineCount(JOINT_TRIPS_FILE) - 1;
string DEPART_PROBABILITY_FILE = "C:\\ITS\\ProbabilityLookup.txt"; // Shuld be the same for entire scenario run (calibrating based on the spatial-temporal distribution of the survey data)
DepartProbability* departprobs;

//Sharing algorithm variables:
bool Maximize = true; //Switches which of the following two values is used
int MinPeople = 2;	//If minimizing, all groups will grow to this size and stop
int MaxPeople = 5;	//If maximizing, all groups will grow to this size and stop
int MaxSharingTimeDifference = 5;// 60; //How many minutes apart two shared trips can be

//Tour requirements
float TourDoableRequirement = 0.5;	//For some legs of a tour to be shared, at least this percent must be doable 
int DrivingModes[] = { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //Each ridesharing group must have one person whose mode is one of these
int DoableTripModes[] = { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };  //These modes do not require the trip to be shared for it to be doable

//Trip sharing requirements:  (ordered by computational complexity)

double CLOSE_DISTANCE = 1;//10	//Two points must be within this to be considered closePoints. Make sure to update vector reserve() calls when changing this
int MaxNumStops = 6;	//Number of stops must be less than or equal to this
int MaxIncome = 1000000; //Income must be below this
float MinDistanceTraveled = 5;	//Distance between origin and dest. must be above this
int TripModes[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; //1 represents that array index is shareable. Right now, indexes modes 1-7 (and not 0) are shareable
unsigned int RandomFailChance = 80; //%chance a trip will randomly not be shareable. This should be an integer from 0-100 
set<string> TripPurposes = { "Home", "work_low", "work_med", "work_high", "work_very high", "university", "school_high", "school_grade", "atwork_business", "atwork_eat", "atwork_maint", \
"eatout", "escort_kids", "escort_no kids", "othdiscr", "othmaint", "shopping", "social" }; //Simply list acceptable purposes here

//Server settings
string DISTANCE_FILE = "D:\\MTC_BASE\\database\\DistanceSkimsDatabaseAM.csv";
string HOUSEHOLD_FILE = "D:\\MTC_BASE\\main\\householdData_4.csv"; //TODO: Fill this in (Simulated or Synthesized)
string PERSON_FILE = "D:\\MTC_BASE\\popsyn\\personFile.p2011s3a.2010.csv";// Synthesized works in this context
string TOUR_FILE = "D:\\MTC_BASE\\main\\indivTourData_4.csv";
string TRIP_FILE = "D:\\MTC_BASE\\main\\indivTripData_4.csv";



int NUM_LOCATIONS = 1454;		//Number of locations in the simulation
int DISTANCE_FILE_SIZE = NUM_LOCATIONS * NUM_LOCATIONS;

int HOUSEHOLD_FILE_SIZE = lineCount(HOUSEHOLD_FILE) - 1; //TODO: record this number
int PERSON_FILE_SIZE = lineCount(PERSON_FILE) - 1;//7053334
int TOUR_FILE_SIZE = lineCount(TOUR_FILE) - 1;//8914778
int TRIP_FILE_SIZE = lineCount(TRIP_FILE) - 1;//22811684
/*
int PERSON_FILE_SIZE = 7053334;
int TOUR_FILE_SIZE = 8914778;
int TRIP_FILE_SIZE = 22811684;
*/

struct Tour; struct Trip; struct Person;

//Data allocation:
Household* all_households;
//vector<Person*> all_people;
Person* all_people;
Tour* all_tours;
Trip* all_trips;
Tour* all_joint_tours;
Trip* all_joint_trips;
vector<short>* closePoints;
bool close[1455][1455];
vector<Trip*>* organized;
//d3* organized;
float* dist;

int shareable = 0;	//Trips that passed the initial checks
int potentialSharing = 0;//Trips with at least one trip it could potentially share with	
int sharingBeforeTourLevel = 0;//Trips that actually shared before tour-level checks
int sharingBeforeReshare = 0;//Trips that actually shared before re=sharing
int actualSharing = 0;//Trips that actually shared with at least one other trip
int unshared = 0;//Trips unshared because of tour-level requirements
int groups = 0;//Number of sharing-groups
int solo = 0;//Trips that could not be actually shared (but weren't unshared)
int orphaned = 0;//Trips that were sharing with a trip that was unshared, and now are not sharing
double VMTReduction = 0; //Vehicle miles saved

bool sortPotentialSharing = false;
double sharingRequirement = -9999;

int g_seed = 1; //Random seed. This determines the set of random numbers generated.

