#ifndef DISTANCES_H
	#define DISTANCES_H

#include "stdafx.h"
#include "Globals.h"

#include <string>
//using namespace std;


//Parses the distances file. Construct with Distances dist(filename), use: dist(start, end)
class Distances
{
private:
	//internal distances array
	float *d;

public:
	//Loads and parses filename
	Distances(string filename);

	//Destructor
	~Distances();

	//() to access distance between arg1 and arg2
	inline float operator()(int arg1, int arg2)
	{
		return d[((arg1 - 1) * NUM_LOCATIONS) + (arg2 - 1)];
	}

	//[]  to access distances array directly
	inline float operator[](int arg)
	{
		return d[arg];
	}
};

#endif