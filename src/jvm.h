#pragma once
#include "class.h"
#include "memery.h"
#include <csignal>
#include <vector>
#include <string>
#include <cstdio>
#include <stdint.h>
#include "thread.h"
#include <unordered_map>

//for jni
#include <dlfcn.h>
#include <ffi.h>


struct vm_native
{
	void * handle = nullptr;
	using native_metod = void(*)();
	vm_native():handle(dlopen(nullptr, RTLD_LAZY)) {}
	static std::string trans_method_name(method * m);
	native_metod find_native_implement(method * m);
};



class jvm
{
	classloader * cloader = nullptr;
	public:
	std::vector<thread *> threads;
	void on_global_signal(int, siginfo_t*, void*);
	void on_signal(int sig);
	vm_native vm_native_methods;
	thread * new_thread();
	jvm();
	classloader * get_class_loader() { return cloader; }
	void init_baisc_type();
	void init(thread * current_thread);
	~jvm();
	void load_runtime(const std::string & runtime_path);
	void run(const std::vector<std::string> & args);

};
