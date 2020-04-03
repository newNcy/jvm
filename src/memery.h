#pragma once
#include <stdint.h>
#include <typeinfo>
#include "class.h"
#include "object.h"
const static uint64_t K = 1024;
const static uint64_t M = 1024*K;
const static uint64_t G = 1024*M;
const jreference null = 0;


class memery
{
	public:
	template <typename T>
	static T * alloc_meta() 
	{
		//printf("alloc %lu bytes\n", sizeof(T));
		return new T;
	}
	template <typename T>
	static void dealloc_meta(T * t)
	{
		delete t;
	}

	static void * alloc_meta_space(size_t size)
	{
		return new char[size]();
	}
	
	template <typename T>
	static void dealloc_meta_space(T * t)
	{
		delete [] (char*)t;
	}


	static std::map<jreference, object*> ref_oop_map;
	static object * ref2oop(jreference ref);
	static jreference alloc_heap_object(claxx * meta);
	static jreference alloc_heap_array(claxx * meta, size_t length);
	static char * alloc_static_members(claxx * meta);
};

