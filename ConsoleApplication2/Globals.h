#ifndef GLOBALS_H
	#define GLOBALS_H

#include "DataClasses.h"

#include<string>
using namespace std;

const static double CLOSE_DISTANCE = .25;
const static int NUM_LOCATIONS = 1454;

const static string DISTANCE_FILE = "I:\\DistanceSkimsDatabaseAM2.csv";
const static int DISTANCE_FILE_SIZE = NUM_LOCATIONS * NUM_LOCATIONS;

const static string PERSON_FILE = "I:\\personFile.p2011s3a.2010.csv";
const static int PERSON_FILE_SIZE = 7053334;

const static string TOUR_FILE = "I:\\indivTourData_3.csv";
const static int TOUR_FILE_SIZE = 8914778;//9029029


const static string TRIP_FILE = "I:\\indivTripData_3.csv";
const static int TRIP_FILE_SIZE = 22811684;//23067850

static Person* all_people;
static Tour* all_tours;
static Trip* all_trips;
static vector<short>* closePoints;
static vector<Trip*>* organized[24];


#endif