#pragma once



#include "class.h"
#include <map>
#include <string>
class jvm;

using fieldID = void*;
using methodID = void*;


class environment
{ 
	friend class jvm;
	private:
		jvm * vm = nullptr;
		thread * current_thread = nullptr;
		std::map<std::string, jreference> interns;
	public:
		environment(jvm * this_vm, thread * this_thread):vm(this_vm), current_thread(this_thread) {}
		jvm * get_vm() { return vm; }
		thread * get_thread() { return current_thread; }
		
		jreference create_string(const std::string & bytes);
		jreference create_string_intern(const std::string & bytes);
		jreference string_intern(jreference jref);

		jint array_length(jreference ref);

		void set_array_element(jreference ref, jint index, jvalue v);

		jvalue get_array_element(jreference ref, jint index);
		
		void set_object_field(jreference ref, fieldID f, jvalue v);

		jvalue get_object_field(jreference ref, fieldID  f);

		jreference create_basic_array(jtype type, uint32_t length);
		jreference create_obj_array(jreference type, uint32_t length);
		jreference get_class(jreference obj);

		fieldID lookup_field(jreference cls, const std::string & name);
		methodID lookup_method(jreference cls, const std::string & name, const std::string & dis);

		jvalue callmethod(methodID m, jreference obj);
};

#define NATIVE extern "C"
