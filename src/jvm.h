#pragma once
#include "class.h"
#include "memery.h"
#include <csignal>
#include <stdexcept>
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

struct well_know_class_name 
{
	const char * name = nullptr;
	claxx * c = nullptr;
	thread * use_this_to_load = nullptr;
	well_know_class_name(const char * n):name(n) {}
	claxx * Class(thread * t = nullptr)
	{
		if (c) return c;
		if (t != nullptr) {
			use_this_to_load = t;
		}
		if (!use_this_to_load) throw std::runtime_error("make sure use thread to get first time");
		return c = t->get_env()->bootstrap_load(name);
	}
};

static well_know_class_name java_lang_Class("java/lang/Class");
static well_know_class_name java_lang_Object("java/lang/Object");
static well_know_class_name java_lang_String("java/lang/String");
static well_know_class_name java_lang_Thread("java/lang/Thread");
static well_know_class_name java_lang_System("java/lang/System");

class jvm
{
	classloader * cloader = nullptr;
	public:
	std::map<std::string, jreference> primitive_types;
	jreference thread_group = null;
	std::vector<thread *> threads;
	void on_global_signal(int, siginfo_t*, void*);
	void on_signal(int sig);
	vm_native vm_native_methods;
	thread * new_thread(jreference m = 0);
	jvm();
	classloader * get_class_loader() { return cloader; }
	void init_baisc_type();
	void init(thread * current_thread);
	~jvm();
	void load_jar(const std::string & runtime_path);
	void run(const std::vector<std::string> & args);

};
