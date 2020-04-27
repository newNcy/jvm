#include <csignal>
#include <cstdio>
#include <ffi-x86_64.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "class.h"
#include "frame.h"
#include "jvm.h"
#include <signal.h>

jvm vm;

void sighandle(int s)
{
	int tid = 0;
	for (auto t : vm.threads) {
		printf("thread [%d]\n", tid++);
		int fid = 0;
		while (t->current_frame) {
			printf("# %d pc[%d]\t%s.%s %s\n", fid ++, 
					t->current_frame->current_pc,
					t->current_frame->current_method->owner->name->c_str(),
					t->current_frame->current_method->name->c_str(),
					t->current_frame->current_method->discriptor->c_str()
				  );
			t->current_frame->print_stack();
			t->current_frame->print_locals();
			t->pop_frame();
		}
	}
	exit(0);
}


int main(int argc, char * argv[])
{
	const char * main_class_name = "Test";
	std::vector<std::string> args;
	args.push_back(main_class_name);


	//signal(SIGSEGV, sighandle);
	//signal(SIGABRT, sighandle);
#if 1
	vm.load_jar("charsets.jar");
	vm.load_jar("rt.jar");

	vm.run(args);
#endif
	return 0;
}
