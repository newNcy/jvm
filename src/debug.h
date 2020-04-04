#pragma once

#include <ctime>
#include <string>
#include <sys/select.h>
class function_time
{
	std::string name;
	timeval t;
	public:
	function_time(const std::string & n):name(n) 
	{
		gettimeofday(&t, nullptr);
	}
	~function_time()
	{
		timeval end;
		gettimeofday(&end, nullptr);
	}
};
