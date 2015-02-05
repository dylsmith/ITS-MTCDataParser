#include "stdafx.h"
#include "ClosePoints.h"
#include "Distances.h"
#include "Timer.h"

#include <string>
#include <iostream>
using namespace std; 

ClosePoints::ClosePoints(Distances &dist)
{
	Timer timeit("Generating ClosePoints");

	int p = 0;
	//Check non-diagonal points where k > i. Graph is symmetric, so we don't need to check k < i
	for (int i = 1; i <= NUM_LOCATIONS; i++)
	{
		for (int k = i + 1; k <= NUM_LOCATIONS; k++)
		{
			if (dist(i, k) < CLOSE_DISTANCE)
			{

				v[i-1].push_back(k);
				v[k-1].push_back(i);
			}
		}
	}

	//Check diagonal points (k = i)
	for (int i = 1; i <= NUM_LOCATIONS; i++)
		if (dist(i, i) < CLOSE_DISTANCE)
			v[i-1].push_back(i);
}

ClosePoints::~ClosePoints()
{

}

//Returns a vector of points close to arg
vector<short> ClosePoints::operator[](int arg)
{
	return v[arg - 1];	//Points start at 1, but arrays start at 0, so fix that here
}