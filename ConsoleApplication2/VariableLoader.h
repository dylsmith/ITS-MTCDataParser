#ifndef VARLOADER_H
#define VARLOADER_H

#include "stdafx.h"
#include "Globals.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>

#include <typeinfo>
#include <tchar.h>
#include <windows.h>


using namespace std;


#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


class VariableLoader
{
	ifstream inf;

public: 
	VariableLoader()
	{
		wstring root = path.substr(0, path.length() - 7) + L"inputs.txt";
		wcout << "Searching for input file in " << root << endl;
		inf.open(root);
		if (!inf.good())
		{
			cout << "Failed to load input file! Exiting." << endl;
			string s;
			cin >> s;
			exit(0);
		}
	}		

	template<typename T>
	T loadVal(string arg)
	{
		inf.clear();
		inf.seekg(0, ios::beg);

		string var;
		T val;
		stringstream ss;
		string str;
		while (getline(inf,str))
		{
			if (str.substr(0, 2) != "//")
			{
				ss = (stringstream)str;
				ss >> var >> val;
				if (arg == var)
				{
					//cout << typeid(val).name() << " " << var << " = " << val << endl;
					if(debug) cout << var << " = " << val << endl;
					return val;
				}
			}
		}

		cout << "Failed to find " << arg << endl;
		return 0;
	}

	template<typename T, int I>
	void loadArray(string arg, array<T, I>& arr)
	{
		inf.clear();
		inf.seekg(0, ios::beg);

		string var;
		stringstream ss;
		string str;
		while (getline(inf, str))
		{
			if (str.substr(0, 2) != "//")
			{
				ss = (stringstream)str;
				ss >> var;
				if (arg == var)
				{
					if(debug) cout << typeid(arr).name() << " " << var << " = ";
					//cout << var << "[" << I << "]" << " = ";
					for (int i = 0; i < arr.size(); i++)
					{
						ss >> arr[i];
						if(debug) cout << arr[i] << " ";
					}
					if(debug) cout << endl;
					return;
				}
			}
		}
		cout << "Failed to find " << arg << endl;
	}
};


void load()
{
	Timer t("Loading input variables");
	VariableLoader v;


	debug = v.loadVal<int>("debug");
	ExecutionMode = v.loadVal<int>("ExecutionMode");
	largeCalculations = v.loadVal<bool>("largeCalculations");

	rideShareWeight = v.loadVal<double>("rideShareWeight");
	householdInteractionWeight = v.loadVal<double>("householdInteractionWeight");
	mandatoryTripWeight = v.loadVal<double>("mandatoryTripWeight");
	nonMandatoryTripWeight = v.loadVal<double>("nonMandatoryTripWeight");
	PercentTripsToShare = v.loadVal<double>("PercentTripsToShare");
	sharingRequirementStep = v.loadVal<double>("sharingRequirementStep");

	minOriginZone = v.loadVal<int>("minOriginZone");
	maxOriginZone = v.loadVal<int>("maxOriginZone");
	minDestinationZone = v.loadVal<int>("minDestinationZone");
	maxDestinationZone = v.loadVal<int>("maxDestinationZone");

	EVAverageRange = v.loadVal<int>("EVAverageRange");
	v.loadArray<int>("EVTripModes", EVTripModes);


	JOINT_TOURS_FILE = v.loadVal<string>("JOINT_TOURS_FILE");
	JOINT_TRIPS_FILE = v.loadVal<string>("JOINT_TRIPS_FILE");
	DEPART_PROBABILITY_FILE = v.loadVal<string>("DEPART_PROBABILITY_FILE");

	Maximize = v.loadVal<bool>("Maximize");
	MinPeople = v.loadVal<int>("MinPeople");
	MaxPeople = v.loadVal<int>("MaxPeople");
	MaxSharingTimeDifference = v.loadVal<int>("MaxSharingTimeDifference");

	TourDoableRequirement = v.loadVal<double>("TourDoableRequirement");
	v.loadArray<int>("DrivingModes", DrivingModes);
	v.loadArray<int>("DoableTripModes", DoableTripModes);


	CLOSE_DISTANCE = v.loadVal<double>("CLOSE_DISTANCE");
	MaxNumStops = v.loadVal<int>("MaxNumStops");
	MaxIncome = v.loadVal<int>("MaxIncome");
	MinIncome = v.loadVal<int>("MinIncome");
	MinDistanceTraveled = v.loadVal<double>("MinDistanceTraveled");
	MaxDistanceTraveled = v.loadVal<double>("MaxDistanceTraveled");
	v.loadArray<int>("TripModes", TripModes);
	RandomFailChance = v.loadVal<unsigned int>("RandomFailChance");
	v.loadArray<int>("TripPurposes", TripPurposes);
	v.loadArray<int>("TourPurposes", TourPurposes);

	v.loadArray<int>("viableHouseholdTypes", viableHouseholdTypes);
	householdIncomeMax = v.loadVal<int>("householdIncomeMax");
	householdVehiclesMax = v.loadVal<int>("householdVehiclesMax");
	v.loadArray<int>("validSizeCat", validSizeCat);
	v.loadArray<int>("validhfamily", validhfamily);
	v.loadArray<int>("validhchildren", validhchildren);
	v.loadArray<int>("validhworker", validhworker);

	maxAge = v.loadVal<int>("maxAge");

	v.loadArray<int>("validESR", validESR);
	v.loadArray<int>("validSex", validSex);
	v.loadArray<int>("validMSP", validMSP);
	v.loadArray<int>("validPTYPE", validPTYPE);
	v.loadArray<int>("validPEmploy", validPEmploy);



	DISTANCE_FILE = v.loadVal<string>("DISTANCE_FILE");
	HOUSEHOLD_FILE = v.loadVal<string>("HOUSEHOLD_FILE");
	PERSON_FILE = v.loadVal<string>("PERSON_FILE");
	TOUR_FILE = v.loadVal<string>("TOUR_FILE");
	TRIP_FILE = v.loadVal<string>("TRIP_FILE");

	ALL_TRIP_DETAILS_FILE = v.loadVal<string>("ALL_TRIP_DETAILS_FILE");
	DATA_FILE = v.loadVal<string>("DATA_FILE");
	TRIP_SHARING_FILE = v.loadVal<string>("TRIP_SHARING_FILE");
	SHARED_DETAILS_FILE = v.loadVal<string>("SHARED_DETAILS_FILE");
	UNSHARED_DETAILS_FILE = v.loadVal<string>("UNSHARED_DETAILS_FILE");
	SHARED_PERSON_FILE = v.loadVal<string>("SHARED_PERSON_FILE");
	UNSHARED_PERSON_FILE = v.loadVal<string>("UNSHARED_PERSON_FILE");
	HOUSEHOLD_EV_FILE = v.loadVal<string>("HOUSEHOLD_EV_FILE");

	WriteInducedDemand = v.loadVal<int>("WriteInducedDemand ");
	WriteTripSharing = v.loadVal<int>("WriteTripSharing");
	WriteTripDetails = v.loadVal<int>("WriteTripDetails");

	
	JOINT_TOURS_FILE_SIZE = lineCount(JOINT_TOURS_FILE) - 1;
	JOINT_TRIPS_FILE_SIZE = lineCount(JOINT_TRIPS_FILE) - 1;
	HOUSEHOLD_FILE_SIZE = lineCount(HOUSEHOLD_FILE) - 1;
	PERSON_FILE_SIZE = lineCount(PERSON_FILE) - 1;//7053334
	TOUR_FILE_SIZE = lineCount(TOUR_FILE) - 1;//8914778
	TRIP_FILE_SIZE = lineCount(TRIP_FILE) - 1;//22811684
	
}

#endif