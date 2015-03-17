#ifndef FASTRAND_H
#define FASTRAND_H

#include "stdafx.h"
#include "Globals.h"

static unsigned long x = 123456789, y = 362436069, z = 521288629;

//unsigned long fastrand(void);
extern int g_seed;

inline int fastrand() {
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}

#endif