#include "stdafx.h"
#include "Timer.h"

#include <iostream>

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
	cout << m_name << ": " << secs << "secs." << endl;
}
