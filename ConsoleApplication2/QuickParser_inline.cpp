#include "stdafx.h"
#include "QuickParser.h"

#include<fstream>
#include <iostream>

//returns the read symbol (if necessary) and parses to one-past the named symbol
inline void QuickParser::parseComma()
{
	while (*(++loc) != ',');
	/*
	while (*loc != ',')// && *loc != '\n' && *loc != '\n')
		loc++;
	//if (*loc != ',')
	//	cout << "Parsed over a newline" << endl;
	loc++;*/
}
inline void QuickParser::parseNewLine()
{
	while (*(++loc) != '\n');
	/*
	while (*loc != '\n')// && *loc != '\r' && *loc != '\0')
		loc++;
	loc++;*/

}
inline float QuickParser::parseFloat()
{
	++loc;
	float ret = strtof(loc, &loc);
	return ret;
}
inline int QuickParser::parseInt()
{
	++loc;
	int ret = strtol(loc, &loc, 10);
	return ret;
}
inline char* QuickParser::parseString()
{
	int i = 0;
	while ((loc)[++i] != ',' && (loc)[i] != '\n');
	char* ret = new char[i + 1];
	memcpy(ret, loc, i);
	ret[i] = '\0';
	loc += i + 1;
	return ret;
}