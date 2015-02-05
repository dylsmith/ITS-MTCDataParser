#include "stdafx.h"

#include "Distances.h"
#include "QuickParser.h"
#include "QuickParser_inline.cpp"
#include "Timer.h"

#include <Windows.h>

#include<iostream>
using namespace std;

//Loads and parses filename
Distances::Distances(string filename)
{
	QuickParser q(filename);
	Timer timeit("Parsing distances");
	d = new float[DISTANCE_FILE_SIZE];
	int i = 0;
	while (i < DISTANCE_FILE_SIZE)
	{
		q.parseComma();
		q.parseComma();

		//OutputDebugString(itow_s(i));
		d[i++] = q.parseFloat();


	}

	//cout << d[0] << endl;
}

Distances::~Distances()
{
	delete[] d;
}

