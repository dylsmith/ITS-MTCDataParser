
#include "stdafx.h"
#include <set>
#include <string>
#include <vector>
#include "MiscFunctions.h"
#include <array>
#include <mutex>
struct DepartProbability;
using namespace std;

int ExecutionMode = 2; // 0 = ridesharing, 1 = EV
bool largeCalculations; //If true, will determine trip sharing on-the-fly rather than saving sharing sets in memory (This is good for scenarios with closedistance greater than 2.5)

//Induced Person Demand
double rideShareWeight; // total inividual weighting factor 
double householdInteractionWeight; // total hh weighting factor
double mandatoryTripWeight;
double nonMandatoryTripWeight;
double PercentTripsToShare; // 10% of trips are ridesharable (fixing them)
double sharingRequirementStep; // 3% decremental step

//EV algorithm variables:
double EVAverageRange; //100 -> 92.9% //EV range
array<int, 19> EVTripModes; //Identical to drivingmodes right now
string JOINT_TOURS_FILE = "D:\\MTC_BASE\\main\\jointTourData_4.csv";
int JOINT_TOURS_FILE_SIZE;
string JOINT_TRIPS_FILE = "D:\\MTC_BASE\\main\\jointTripData_4.csv";
int JOINT_TRIPS_FILE_SIZE;
string DEPART_PROBABILITY_FILE = "C:\\ITS\\ProbabilityLookup.txt"; // Shuld be the same for entire scenario run (calibrating based on the spatial-temporal distribution of the survey data)
DepartProbability* departprobs;

//Sharing algorithm variables:
bool Maximize; //Switches which of the following two values is used
int MinPeople;	//If minimizing, all groups will grow to this size and stop
int MaxPeople;	//If maximizing, all groups will grow to this size and stop
int MaxSharingTimeDifference;// 60; //How many minutes apart two shared trips can be

//Tour requirements
float TourDoableRequirement;	//For some legs of a tour to be shared, at least this percent must be doable 
array<int, 19> DrivingModes; //Each ridesharing group must have one person whose mode is one of these
array<int, 19> DoableTripModes;  //These modes do not require the trip to be shared for it to be doable

//Trip sharing requirements:  (ordered by computational complexity)
double CLOSE_DISTANCE;//double//Two points must be within this to be considered closePoints. Make sure to update vector reserve() calls when changing this
int MaxNumStops;	//Number of stops must be less than or equal to this
int MaxIncome; //Income must be below this ## maximum ind income is 375k
float MinDistanceTraveled;	//Distance between origin and dest. must be above this
//                  0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18
array<int, 19> TripModes; //1 represents that array index is shareable. Right now, indexes modes 1-7 (and not 0) are shareable
unsigned int RandomFailChance; //%chance a trip will randomly not be shareable. This should be an integer from 0-100 
array<int, 18> TripPurposes; //Simply list acceptable purposes here

// Household 
array<int, 8> viableHouseholdTypes; //fill this in with 1's for valid household types and 0's for the rest
double householdIncomeMax;
int householdVehiclesMax;


//Person restrictions
int maxAge;
array<int, 7> validESR; // Employment Status
array<int, 3> validSex; //1 = male, 2 = female
array<int, 7> validMSP ; // Marital Status
array<int, 9> validPTYPE ; // Person type



//Server settings
string DISTANCE_FILE;
string HOUSEHOLD_FILE;
string PERSON_FILE;// Synthesized works in this context
string TOUR_FILE;
string TRIP_FILE;

// Farzad Output settings
string PERSON_DETAILS_FILE; //Person file output
string ALL_TRIP_DETAILS_FILE; //All trips printed out here, with modes changed to '5' if in a group
string DATA_FILE;//Important data points written here
string TRIP_SHARING_FILE;//Each tripid and its actual sharing list will be written to this file
bool WriteTripSharing;
string TRIP_DETAILS_FILE;	//Shared trips will be merged (and mode changed to 5), unshared trips left intact, and written to this
string SHARED_DETAILS_FILE; //Split versions of above
string UNSHARED_DETAILS_FILE;
string SHARED_PERSON_FILE; // person file for induced demand 
bool WritePersonDetails;
string UNSHARED_PERSON_FILE; // person file for induced demand
string HOUSEHOLD_EV_FILE;
bool WriteTripDetails;

/*
//Dylan Output settings
string PERSON_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\PersonOutput.csv"; //Person file output
string ALL_TRIP_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\AllTripDetails.csv"; //All trips printed out here, with modes changed to '5' if in a group
string DATA_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\DataPoints.txt";//Important data points written here
string TRIP_SHARING_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\TripSharing.txt";//Each tripid and its actual sharing list will be written to this file
bool WriteTripSharing = false;
string TRIP_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\TripDetails.csv";	//Shared trips will be merged (and mode changed to 5), unshared trips left intact, and written to this
string SHARED_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\SharedTripDetails.csv"; //Split versions of above
string UNSHARED_DETAILS_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\UnsharedTripDetails.csv";
string SHARED_PERSON_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\SharedPersonDetails.csv"; // person file for induced demand 
bool WritePersonDetails = false;
string UNSHARED_PERSON_FILE = "D:\\MTC_BASE\\PostProcess\\Sensitivity\\test\\UnsharedPersonDetails.csv"; // person file for induced demand
string HOUSEHOLD_EV_FILE = "D:\\MTC_BASE\\PostProcess\\HouseholdEVData_100.csv";
bool WriteTripDetails = false;
*/

//Internals
int NUM_LOCATIONS = 1454;		//Number of locations in the simulation
int DISTANCE_FILE_SIZE = NUM_LOCATIONS * NUM_LOCATIONS;

int HOUSEHOLD_FILE_SIZE; 
int PERSON_FILE_SIZE;//7053334
int TOUR_FILE_SIZE;//8914778
int TRIP_FILE_SIZE;//22811684
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
double householdIncome = 0; //Average household income
double householdType = 0; 
double householdVehicles = 0; //Average household vehicles
int viableHouseholds = 0;

bool sortPotentialSharing = false;
double sharingRequirement = -9999;

int g_seed = 1; //Random seed. This determines the set of random numbers generated.
int debug;

wstring path;

