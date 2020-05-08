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
	thread(jvm * this_vm, jreference m = null );
	environment * get_env() { return &runtime_env; }
	jreference create_string(const std::string & bytes);
	jvalue call(method * m,array_stack * args = nullptr);
	template <typename ... Args>
	jvalue call(method * m, Args ... rags);
	void pop_frame();
	void run();
	bool handle_exception();
	void throw_exception_to_java(const std::string & name);
	bool is_daemon();
	void start();
	int depth = 1;
	bool finish = false;


};

template <typename ... Args>
jvalue thread::call(method * m, Args ... args)
{
	int args_size = 0;
	for (auto arg : m->arg_types) {
		if (type_size[arg] > sizeof(int)) args_size ++;
		args_size ++;
	}

	array_stack arg_pack(args_size+1);
	arg_pack.push<element_type<Args>...>(args...);
	this->call(m, &arg_pack);
}
