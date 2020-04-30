#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <sys/time.h>
#include <stack>
#include "attribute.h"
#include "class.h"
#include "frame.h"
#include "memery.h"
#include "native.h"


struct thread;
typedef jint local_entry;
struct jvm;
struct thread
{
	jreference mirror = null;
	frame			* current_frame = nullptr;
	std::vector<frame*> abrupt_frames;
	environment runtime_env;
	thread(jvm * this_vm);
	environment * get_env() { return &runtime_env; }
	jreference create_string(const std::string & bytes);
	jvalue call(method * m,array_stack * args = nullptr);
	void pop_frame();
	void run();
	bool handle_exception();
	void throw_exception_to_java(const std::string & name);
	void start() 
	{
		run();
	}
	int depth = 1;
};



