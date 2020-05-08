#include "jvm.h"
#include "class.h"
#include "classloader.h"
#include "frame.h"
#include "object.h"
#include "thread.h"
#include <csignal>
#include <cstdio>
#include <dlfcn.h>
#include <sstream>
#include <stdexcept>
#include "log.h"

jvm::jvm():cloader(memery::alloc_meta<classloader>()) 
{
}

void jvm::load_jar(const std::string & runtime_path)
{
	zip * z = zip_open(runtime_path.c_str(), 0, nullptr);
	if (!z) {
		throw std::runtime_error("jar not found");
	}
	cloader->jars.push_back(z);
	//cloader->load_jar(runtime_path);
}

thread * jvm::new_thread(jreference m)
{
	thread * ret = new thread(this, m);

	if (!thread_group) {
		claxx * java_lang_ThreadGroup = cloader->load_class("java/lang/ThreadGroup", ret);
		thread_group = java_lang_ThreadGroup->instantiate(ret);
	}
	auto f = ret->get_env()->lookup_field_by_class(ret->get_env()->get_class(ret->mirror), "group");
	ret->get_env()->set_object_field(ret->mirror, f, thread_group);
	threads.push_back(ret);
	return ret;
}

std::string vm_native::trans_method_name(method * m)
{
	if (!m) return "";
	std::stringstream full_name;
	std::string cls_name = m->owner->name->c_str();
	for (auto & c : cls_name ) {
		if (c == '/') c = '_';
		if (c == '$') c = '_';
	}
	full_name<<cls_name<<'_'<<m->name->c_str();

	return full_name.str();
}

vm_native::native_metod vm_native::find_native_implement(method * m)
{
	if (!m) return 0;
	const std::string & full_name = trans_method_name(m);
	//printf("find native %s\n", full_name.c_str());
	return (native_metod)dlsym(handle, full_name.c_str());
}

void jvm::init_baisc_type()
{
#define PRIMITIVE(t) primitive_types[type_text[t]] = cloader->create_primitive(t)
	PRIMITIVE(T_VOID);
	PRIMITIVE(T_BOOLEAN);
	PRIMITIVE(T_CHAR);
	PRIMITIVE(T_FLOAT);
	PRIMITIVE(T_DOUBLE);
	PRIMITIVE(T_BYTE);
	PRIMITIVE(T_SHORT);
	PRIMITIVE(T_INT);
	PRIMITIVE(T_LONG);
#undef PRIMITIVE
}
void jvm::init(thread * current_thread)
{
	claxx * class_class = java_lang_Class.Class(current_thread);
	init_baisc_type();
	claxx * system_class = java_lang_System.Class(current_thread);
	method * init_system_class = system_class->lookup_method("initializeSystemClass", "()V");
	cloader->initialize_class(system_class, current_thread);
	if (!init_system_class) return;
	current_thread->call(init_system_class);
	claxx * string_class = java_lang_String.Class( current_thread);
}

void jvm::run(const std::vector<std::string> & args)
{
	if (args.empty()) return;
	thread * main_thread = new_thread();	


	init(main_thread);
	claxx * main_class = cloader->load_class(args[0], main_thread);
	if (main_class->state < INITED) cloader->initialize_class(main_class, main_thread);
	if (!main_class) {
		printf("can't not load main class\n");
		return;
	}
	method * main_method = main_class->lookup_method("main", "([Ljava/lang/String;)V");
	if (!main_method) {
		printf("can't not find main method\n");
		return;
	}

	if (!main_method->is_public() || !main_method->is_static()) {
		printf("main method should be public and static\n");
		return;
	}
	
	array_stack args_stack(1); // String [] args
	auto s = cloader->load_class("java/lang/String", main_thread);
	auto sa = s->get_array_claxx(main_thread);

	jreference jargs = sa->instantiate(args.size() - 1, main_thread);
	for (int i = 1 ; i < args.size(); i++) {
		jreference str = main_thread->get_env()->create_string_intern(args[i]);
		main_thread->get_env()->set_array_element(jargs, i-1, str);
	}
	main_thread->call(main_method, jargs);
	bool all_done = false;
	while (!all_done) {
		all_done = true;
		for (auto it = threads.begin() ; it != threads.end(); ){
			if ((*it)->is_daemon()) continue;
			if ((*it)->finish) {
				delete *it;
				it = threads.erase(it);
			}else {
				++ it;
				all_done = false;
			}
		}
	}
} 

jvm::~jvm()
{
	for (auto & t : threads) {
		if (t) {
			delete t;
			t = nullptr;
		}
	}
	threads.clear();
}


