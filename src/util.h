#pragma once
#include <cstdio>
#include <time.h>

struct execute_time
{
	const char * name = nullptr;
	clock_t start_time = 0;
	execute_time(const char * n):name(n),start_time(clock())
	{
	}
	~execute_time()
	{
		if (name) {
			printf("%s execute take %ld\n",name, clock() - start_time);
		}
	}
};
