#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ffi-x86_64.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>
#include "class.h"
#include "classfile.h"
#include "frame.h"
#include "jvm.h"
#include "log.h"
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
			//t->current_frame->print_stack();
			//t->current_frame->print_locals();
			t->pop_frame();
		}
	}
	exit(0);
}


struct ts 
{
	u2 first;
	u2 second;
};
int main(int argc, char * argv[])
{
#if 0
	const char * main_class_name = "Test";
#endif



	//signal(SIGSEGV, sighandle);
#if 1
	if (argc < 2) return 0;
	std::vector<std::string> args;
	for (int i = 1 ; i < argc; i++) {
		args.push_back(argv[i]);
	}
	vm.load_jar("charsets.jar");
	vm.load_jar("rt.jar");

	signal(SIGINT, sighandle);
	vm.run(args);
#else
	
	std::vector<int> is;
	log::trace("%d", is.size());
	is.resize(5);
	log::trace("%d", is.size());
	const char * test_hash[] = {"java/lang/Class.getName0()Ljava/lang/String;", "java/lang/Class.getName1()Ljava/lang/String;"};

	for (auto c : test_hash) {
		log::trace("%s --> %d", c, symbol::hash_for_max(c,2));
	}

	jlong l = 726238594903828561;
	u4 * p = (u4*)&l;
	log::trace("%ld", l);
	log::trace("%ld %d", p[0], p[1]);

	byte_stream bs;
	bs.set_buf((const char*)p, 8);

	u4 H = bs.get<u4>();
	u4 L = bs.get<u4>();

	log::trace("%ld", ((jlong)H << 32) | L);
	log::trace("%ld", ((jlong)p[1] << 32) | p[0]);

	jlong t = 0x0123456789abcdef;
	bs.set_buf((const char*)&t, 8);
	ts p2 = {bs.get<u2>(), bs.get<u2>()};
	log::trace("%02x %02x", p2.first, p2.second);
#endif
	return 0;
}
