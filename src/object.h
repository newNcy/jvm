#pragma once

#include "class.h"
class claxx;

struct object
{
	claxx * meta = nullptr;
	static object * from_reference(jreference ref);
	bool is_static_obj = false;
	int element_count = 0;
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
	object(bool st, int ec = 0): is_static_obj(st), element_count(ec) {}
	object * clone ();
	int size()  
	{
		if (is_static_obj) return meta->static_size();
		if (is_array()) {
			return meta->size(element_count);
		}
		return meta->size();
	}

	int array_length() 
	{
		return element_count;
	}

	jvalue get_field(field * f)
	{
		jvalue ret = 0;
		char * data_addr = data;
		if (f->type == T_BOOLEAN)		ret.z = get<jboolean>(f->offset);
		else if (f->type == T_CHAR)		ret.c = get<jchar>(f->offset); 
		else if (f->type == T_FLOAT)	ret.f = get<jfloat>(f->offset); 
		else if (f->type == T_DOUBLE)	ret.d = get<jdouble>(f->offset); 
		else if (f->type == T_BYTE)		ret.b = get<jbyte>(f->offset); 
		else if (f->type == T_SHORT)	ret.s = get<jshort>(f->offset); 
		else if (f->type == T_INT)		ret.i = get<jint>(f->offset); 
		else if (f->type == T_LONG)		ret.j = get<jlong>(f->offset); 
		else if (f->type == T_OBJECT)	ret.l = get<jreference>(f->offset); 
		return ret;
	}
	void set_field(field * f, jvalue v)
	{
		if (f->type == T_BOOLEAN)		put(f->offset, v.z);
		else if (f->type == T_CHAR)		put(f->offset, v.c);
		else if (f->type == T_FLOAT)	put(f->offset, v.f);
		else if (f->type == T_DOUBLE)	put(f->offset, v.d);
		else if (f->type == T_BYTE)		put(f->offset, v.b);
		else if (f->type == T_SHORT)	put(f->offset, v.s);
		else if (f->type == T_INT)		put(f->offset, v.i);
		else if (f->type == T_LONG)		put(f->offset, v.j);
		else if (f->type == T_OBJECT)	put(f->offset, v.l);
	}

	jvalue get_element(int index)
	{
		jvalue ret = 0;
		if (!is_array()) abort();
		array_claxx * type = dynamic_cast<array_claxx*>(meta);
		if (type->componen_type == T_BOOLEAN)		ret.z = get<jboolean>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_CHAR)		ret.c = get<jchar>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_FLOAT)	ret.f = get<jfloat>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_DOUBLE)	ret.d = get<jdouble>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_BYTE)		ret.b = get<jbyte>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_SHORT)	ret.s = get<jshort>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_INT)		ret.i = get<jint>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_LONG)		ret.j = get<jlong>(index*type_size[type->componen_type]);
		else if (type->componen_type == T_OBJECT)	ret.l = get<jreference>(index*type_size[type->componen_type]);
		return ret;
	}

	void set_element(int index, jvalue e)
	{
		if (!is_array()) abort();
		array_claxx * type = dynamic_cast<array_claxx*>(meta);
		if (type->componen_type == T_BOOLEAN)		put(index*type_size[type->componen_type], e.z);
		else if (type->componen_type == T_CHAR)		put(index*type_size[type->componen_type], e.c);
		else if (type->componen_type == T_FLOAT)	put(index*type_size[type->componen_type], e.f);
		else if (type->componen_type == T_DOUBLE)	put(index*type_size[type->componen_type], e.d);
		else if (type->componen_type == T_BYTE)		put(index*type_size[type->componen_type], e.b);
		else if (type->componen_type == T_SHORT)	put(index*type_size[type->componen_type], e.s);
		else if (type->componen_type == T_INT)		put(index*type_size[type->componen_type], e.i);
		else if (type->componen_type == T_LONG)		put(index*type_size[type->componen_type], e.j);
		else if (type->componen_type == T_OBJECT)	put(index*type_size[type->componen_type], e.l);
	}

	template <typename T> 
		T get(int offset)
		{
			if (offset < 0 || offset >= size()) {
				for (auto f : meta->fields) {
					printf("%s field %s offset %d of size %d %ld\n",meta->name->c_str(), f.second->name->c_str(), f.second->offset, size(), meta->size());
				}
				abort();
			}
			return *(T*)(data + offset);
		}

	template <typename T>
		void put(int offset, T t)
		{
			if (offset < 0 || offset >= size())  {
				printf("offset %d out of %d (%d)\n,", offset, size(), element_count);
				abort();
			}
			*(T*)(data + offset) = t;
		}

	char data[];
};
