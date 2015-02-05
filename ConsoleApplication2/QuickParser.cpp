#include "stdafx.h"
#include "QuickParser.h"
#include "Timer.h"

#include<fstream>
#include<iostream>
using namespace std;

//Sets up a file for parsing
QuickParser::QuickParser(string filename)
{
	Timer timeit("Loading " + filename);
	//Open file
	FILE* fp = new FILE();
	int err = fopen_s(&fp, filename.c_str(), "rb");
	if (fp == NULL)
	{
		cout << "Error " << err << " when opening file!";
		pause();
		exit(0);
	}

	//Read whole thing into *file
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);	//Seek to end, get filesize
	fseek(fp, 0, SEEK_SET);
	file = new char[length];	//Allocate memory and read

	long int read = fread((void*)file, 1, length, fp);

	//Make sure the whole thing was read successfully. Fails easily with bad files.
	if (read != length)
		cout << "QuickParser wanted " << length << "bytes from file, but got " << read << endl;
	fclose(fp);

	loc = file;
}

//Sets up a string for parsing. Use .c_str() when passing
QuickParser::QuickParser(char* test, int p)
{
	cout << "Reading from string" << endl;
	length = strlen(test);
	file = const_cast<char*>(test);
	loc = file;
}

//Destructor 
QuickParser::~QuickParser()
{
	delete file;
}

