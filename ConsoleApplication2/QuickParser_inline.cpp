#include "stdafx.h"
#include "QuickParser.h"

#include<fstream>
#include <iostream>

//returns the read symbol (if necessary) and parses to one-past the named symbol
inline void QuickParser::parseComma()
{
	while (*(++loc) != ',');
}
inline void QuickParser::parseNewLine()
{
	while (*(++loc) != '\n');
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
	//Parses one string and moves loc accordingly. Somehow.
	int i = 1;
	while (loc[i] != ',' && loc[i] != '\n'&& loc[i] != '\r')
		++i;
	++i;
	char* ret = new char[i];
	strncpy_s(ret-1,i, loc, _TRUNCATE);
	ret[i-1] = '\0';
	loc += (i - 1);
	return ret;
}