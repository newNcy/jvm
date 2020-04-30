#pragma once
#include "attribute.h"
#include <time.h>


template <typename T> 
struct attribute_name_helper
{
	static const char * name;
};

#define MAP_ATTR_NAME(ATTR, NAME) \
template <> struct attribute_name_helper<ATTR> { static constexpr const char * name = NAME;}

MAP_ATTR_NAME( enclosing_method_attr,	"EnclosingMethod"	);
MAP_ATTR_NAME( inner_classes_attr,		"InnerClasses"		);

#undef MAP_ATTR_NAME


template <typename T>
const char * attribute_name()
{
	return attribute_name_helper<T>::name;
}


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


