#include "GUtill.h"
#include <random>

int Random(int min, int max)
{
	if (min > max)
		return 0;

#ifndef __FIX_SEED__
	static thread_local std::mt19937 gen(std::random_device{}());
#else
	static thread_local std::mt19937 gen(19937);
#endif // __FIX_SEED__

	std::uniform_int_distribution<> dist(min, max);

	return dist(gen);
}
