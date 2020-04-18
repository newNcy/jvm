#include "jvm.h"
#include "class.h"
#include "classloader.h"
#include "thread.h"
#include <csignal>
#include <cstdio>
#include <dlfcn.h>
#include <sstream>

jvm::jvm():cloader(memery::alloc_meta<classloader>()) 
{
}

void jvm::load_runtime(const std::string & runtime_path)
{
	cloader->runtime_path = runtime_path;
	cloader->rt_jar = zip_open(runtime_path.c_str(), 0, nullptr);
	//cloader->load_jar(runtime_path);
}

thread * jvm::new_thread()
{
	thread * ret = new thread(this);
	if (!thread_group) {
		claxx * java_lang_ThreadGroup = cloader->load_class("java/lang/ThreadGroup", ret);
		thread_group = java_lang_ThreadGroup->instantiate(ret);
	}
	auto f = ret->get_env()->lookup_field(ret->get_env()->get_class(ret->mirror), "group");
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
	}
	full_name<<cls_name<<'_'<<m->name->c_str();

	return full_name.str();
}

vm_native::native_metod vm_native::find_native_implement(method * m)
{
	if (!m) return 0;
	const std::string & full_name = trans_method_name(m);
	printf("find native %s\n", full_name.c_str());
	return (native_metod)dlsym(handle, full_name.c_str());
}

void jvm::init_baisc_type()
{
	cloader->create_primitive(T_VOID);
	cloader->create_primitive(T_BOOLEAN);
	cloader->create_primitive(T_CHAR);
	cloader->create_primitive(T_FLOAT);
	cloader->create_primitive(T_DOUBLE);
	cloader->create_primitive(T_BYTE);
	cloader->create_primitive(T_SHORT);
	cloader->create_primitive(T_INT);
	cloader->create_primitive(T_LONG);
}
void jvm::init(thread * current_thread)
{
	claxx * class_class = cloader->load_class("java/lang/Class", current_thread);
	init_baisc_type();
	claxx * system_class = cloader->load_class("java/lang/System", current_thread);
	method * init_system_class = system_class->lookup_method("initializeSystemClass", "()V");
	cloader->initialize_class(system_class, current_thread);
	if (!init_system_class) return;
	current_thread->call(init_system_class);
	claxx * string_class = cloader->load_class("java/lang/String", current_thread);
}

void jvm::run(const std::vector<std::string> & args)
{
	if (args.empty()) return;
	thread * main_thread = new_thread();	


	init(main_thread);
	printf("----------------------------------------------------------------------------------------------\n");
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
	
	operand_stack args_stack(args.size());
	args_stack.push<jreference>(128);
	main_thread->call(main_method, &args_stack);
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
	if (cloader->rt_jar) zip_close(cloader->rt_jar);
}


