#pragma once
#include "class.h"
#include <time.h>

template <typename T> struct element_type_helper;
template <> struct element_type_helper <jboolean>	{ using type = jint; };
template <> struct element_type_helper <jchar>		{ using type = jint; };
template <> struct element_type_helper <jbyte>		{ using type = jint; };
template <> struct element_type_helper <jshort>		{ using type = jint; };
template <> struct element_type_helper <jint>		{ using type = jint; };
template <> struct element_type_helper <jlong>		{ using type = jlong; };
template <> struct element_type_helper <jfloat>		{ using type = jfloat; };
template <> struct element_type_helper <jdouble>	{ using type = jdouble; };
template <> struct element_type_helper <jreference>	{ using type = jreference; };

template <typename RawType>
using element_type = typename element_type_helper<RawType>::type;

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


