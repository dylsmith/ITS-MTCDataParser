#ifndef QUICKPARSER_H
	#define QUICKPARSER_H

#include "stdafx.h"

#include <string>
using namespace std;

class QuickParser
{
public:
	long int length;

public:
	char* file;
	char* loc;

	//Sets up a file for parsing
	QuickParser(string filename);

	//Sets up a string for parsing. Use .c_str() when passing
	QuickParser(char* test, int p);

	//Destructor 
	~QuickParser();

	//returns the read symbol (if necessary) and parses to one-past the named symbol
	inline void parseComma();
	inline void parseNewLine();
	inline float parseFloat();
	inline int parseInt();
	inline char* parseString();
};

#endif