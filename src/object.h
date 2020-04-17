#pragma once

#include "class.h"
class claxx;


struct object
{
	claxx * meta = nullptr;
	int mem_size = 0;
	int array_size = 0;
	bool is_array() { return meta->is_array(); }
	bool is_instance(claxx * cls) 
	{ 
		claxx * c = meta;
		while (c) {
			if (c == cls) return true;
			c = c->super_class;
		}
		return false;
	}
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
		if (f->type == T_BOOLEAN)		put(data_addr, f->offset, v.z);
		else if (f->type == T_CHAR)		put(data_addr, f->offset, v.c);
		else if (f->type == T_FLOAT)	put(data_addr, f->offset, v.f);
		else if (f->type == T_DOUBLE)	put(data_addr, f->offset, v.d);
		else if (f->type == T_BYTE)		put(data_addr, f->offset, v.b);
		else if (f->type == T_SHORT)	put(data_addr, f->offset, v.s);
		else if (f->type == T_INT)		put(data_addr, f->offset, v.i);
		else if (f->type == T_LONG)		put(data_addr, f->offset, v.j);
		else if (f->type == T_OBJECT)	put(data_addr, f->offset, v.l);
	}

	jvalue get_element(int index)
	{
		jvalue ret = 0;
		if (!is_array()) abort();
		array_claxx * type = dynamic_cast<array_claxx*>(meta);
		if (type->componen_type == T_BOOLEAN)		ret.z = get<jboolean>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_CHAR)		ret.c = get<jchar>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_FLOAT)	ret.f = get<jfloat>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_DOUBLE)	ret.d = get<jdouble>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_BYTE)		ret.b = get<jbyte>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_SHORT)	ret.s = get<jshort>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_INT)		ret.i = get<jint>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_LONG)		ret.j = get<jlong>(data, index*type_size[type->componen_type]);
		else if (type->componen_type == T_OBJECT)	ret.l = get<jreference>(data, index*type_size[type->componen_type]);
		return ret;
	}

	void set_element(int index, jvalue e)
	{
		if (!is_array()) abort();
		array_claxx * type = dynamic_cast<array_claxx*>(meta);
		if (type->componen_type == T_BOOLEAN)		put(data, index*type_size[type->componen_type], e.z);
		else if (type->componen_type == T_CHAR)		put(data, index*type_size[type->componen_type], e.c);
		else if (type->componen_type == T_FLOAT)	put(data, index*type_size[type->componen_type], e.f);
		else if (type->componen_type == T_DOUBLE)	put(data, index*type_size[type->componen_type], e.d);
		else if (type->componen_type == T_BYTE)		put(data, index*type_size[type->componen_type], e.b);
		else if (type->componen_type == T_SHORT)	put(data, index*type_size[type->componen_type], e.s);
		else if (type->componen_type == T_INT)		put(data, index*type_size[type->componen_type], e.i);
		else if (type->componen_type == T_LONG)		put(data, index*type_size[type->componen_type], e.j);
		else if (type->componen_type == T_OBJECT)	put(data, index*type_size[type->componen_type], e.l);
	}

	template <typename T> 
		T get(char * start, int offset)
		{
			if (offset < 0 || offset >= size()) abort();
			return *(T*)(start + offset);
		}

	template <typename T>
		void put(char * start, int offset, T t)
		{
			if (offset < 0 || offset >= size()) abort();
			*(T*)(start + offset) = t;
		}

	char data[];
};
