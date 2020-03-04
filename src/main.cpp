#include <cstdio>
#include <string>
#include <vector>
#include "jvm.h"


#define JCALL 
JCALL void test(int a, int b)
{
}


typedef void(*call)();
int main(int argc, char * argv[])
{
	if (argc < 2) return 0;
	const char * main_class_name = argv[1];
	std::vector<std::string> args;
	for (int i = 1; i < argc; i ++) {
		args.push_back(argv[i]);
	}
	
	jvm vm;
	vm.load_runtime("rt.jar");
	vm.run(args);

	return 0;
}
