#pragma once
#include "memery.h"
template <typename T>
class array
{
	size_t capacity = 0;;
	public:
	array(size_t count) {}
	T data[];
	array * alloc(size_t count)
	{
		array * p = (array*)memery::alloc_meta_space(sizeof(array) + count*sizeof(T));
		new (p) array(count);
		return p;
	}
	void dealloc(array*)
	{
		memery::dealloc_meta_space(p);
	}
};

