#include "stdafx.h"

#include "FastRand.h"
/*
unsigned long fastrand(void)
{
	unsigned long t;
	x ^= x << 16;
	x ^= x >> 5;
	x ^= x << 1;
	t = x;
	x = y;
	y = z;
	z = t ^ x ^ y;
	return z;
}*/

