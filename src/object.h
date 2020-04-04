#pragma once

#include "class.h"
class claxx;


struct object
{
	claxx * meta = nullptr;
	int mem_size = 0;
	int array_size = 0;
	bool is_array() { return meta->is_array(); }
	object(int msize, int arr_size = 0):mem_size(msize), array_size(arr_size) {}
	int size()  
	{
		return mem_size;
	}

	int array_length() 
	{
		return array_size;
	}

	jvalue get_field(field * f)
	{
		jvalue ret = 0;
		char * data_addr = data;
		if (f->type == T_BOOLEAN)		ret.z = get<jboolean>(data_addr, f->offset);
		else if (f->type == T_CHAR)		ret.c = get<jchar>(data_addr, f->offset); 
		else if (f->type == T_FLOAT)	ret.f = get<jfloat>(data_addr, f->offset); 
		else if (f->type == T_DOUBLE)	ret.d = get<jdouble>(data_addr, f->offset); 
		else if (f->type == T_BYTE)		ret.b = get<jbyte>(data_addr, f->offset); 
		else if (f->type == T_SHORT)	ret.s = get<jshort>(data_addr, f->offset); 
		else if (f->type == T_INT)		ret.i = get<jint>(data_addr, f->offset); 
		else if (f->type == T_LONG)		ret.j = get<jlong>(data_addr, f->offset); 
		else if (f->type == T_OBJECT)	ret.l = get<jreference>(data_addr, f->offset); 
		return ret;
	}
	void set_field(field * f, jvalue v)
	{
		char * data_addr = data;
		if (f->type == T_BOOLEAN)		put(data_addr, v.z, f->offset);
		else if (f->type == T_CHAR)		put(data_addr, v.c, f->offset);
		else if (f->type == T_FLOAT)	put(data_addr, v.f, f->offset);
		else if (f->type == T_DOUBLE)	put(data_addr, v.d, f->offset);
		else if (f->type == T_BYTE)		put(data_addr, v.b, f->offset);
		else if (f->type == T_SHORT)	put(data_addr, v.s, f->offset);
		else if (f->type == T_INT)		put(data_addr, v.i, f->offset);
		else if (f->type == T_LONG)		put(data_addr, v.j, f->offset);
		else if (f->type == T_OBJECT)	put(data_addr, v.l, f->offset);
	}

	template <typename T> 
		T get(char * start, int offset)
		{
			if (offset < 0 || offset >= size()) abort();
			return *(T*)(start + offset);
		}

	template <typename T>
		void put(char * start, T t, int offset)
		{
			if (offset < 0 || offset >= size()) abort();
			*(T*)(start + offset) = t;
		}

	char data[];
};
