#ifndef CLOSEPOINTS_H
	#define CLOSEPOINTS_H

#include "stdafx.h"
#include "Distances.h"
#include "Globals.h"

#include <string>
#include <vector>
using namespace std;

//Parses the distances object, making vectors of close points
class ClosePoints
{
private:
	//Maximum distance for two points to be considered close
	vector<short> v[NUM_LOCATIONS];

public:
	//Loads and parses filename
	ClosePoints(Distances &dist);

	//Destructor
	~ClosePoints();

	//Returns a vector of points close to arg
	inline vector<short> operator[](int arg);
};

#endif