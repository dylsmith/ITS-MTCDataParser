#ifndef TIMER_H
	#define TIMER_H

#include "stdafx.h"

#include <ctime>
#include <string>

void pause();

class Timer {
	std::string m_name;
	std::clock_t m_started;
public:
	Timer(const std::string &name = "undef");
	~Timer(void);
};

#endif