#pragma once



#include "class.h"
class jvm;


class environment
{ 
	friend class jvm;
	private:
		jvm * vm = nullptr;
		thread * current_thread = nullptr;
	public:
		environment(jvm * this_vm, thread * this_thread):vm(this_vm), current_thread(this_thread) {}
		jvm * get_vm() { return vm; }
		thread * get_thread() { return current_thread; }
		
		jreference create_string(const std::string & bytes);
		jint array_length(jreference ref);
		void set_object_field(jreference ref, field * f, jvalue v);
		jvalue get_object_field(jreference ref, field * f);
		jreference create_basic_array(jtype type, uint32_t length);

};

#define NATIVE extern "C"
