#include "jvm.h"
#include "classloader.h"


void jvm::load_runtime(const std::string & runtime_path)
{
	cloader->runtime_path = runtime_path;
	cloader->rt_jar = zip_open(runtime_path.c_str(), 0, nullptr);
	//cloader->load_jar(runtime_path);
}

thread * jvm::new_thread()
{
	thread * ret = memery::alloc_meta<thread>();
	ret->vm = this;
	threads.push_back(ret);
	return ret;
}

void jvm::run(std::vector<std::string> args)
{
	if (args.empty()) return;
	thread * main_thread = new_thread();	
	claxx * main_class = cloader->load_class(args[0], main_thread);
	cloader->initialize_class(main_class, main_thread);
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

	main_thread->current_stack.push<jint>(128);
	main_thread->start(main_method); } jvm::~jvm()
{
	if (cloader->rt_jar) zip_close(cloader->rt_jar);
}
