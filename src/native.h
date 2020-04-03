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
		
		jreference new_chararray(uint32_t length);

};

#define NATIVE extern "C"
