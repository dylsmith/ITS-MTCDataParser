#include "stdafx.h"
#include "Timer.h"

#include <iostream>
#include <time.h>
#include <ctime>

using namespace std;

string tod()
{
	time_t now;
	time(&now);
	tm ltm;
	localtime_s(&ltm, &now);
	int hour = ltm.tm_hour;
	if (hour == 0)
		hour = 12;
	return "[" + to_string(hour) + ":" + to_string(ltm.tm_min) + ":" + to_string(ltm.tm_sec) + "] ";
}

void pause()
{
	char line;
	std::cin >> line;
}

// ----- source -----

#include <iostream>
using std::cout;
using std::endl;
using std::clock;

Timer::Timer(const std::string &name)
	: m_name(name), m_started(clock()) {
	// empty
}

Timer::~Timer(void) {
	double secs = static_cast<double>(clock() - m_started) / CLOCKS_PER_SEC;
	cout << tod() << m_name << ": " << secs << "secs." << endl;
}
