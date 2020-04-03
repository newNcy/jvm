#include <cstdio>
#include <ffi-x86_64.h>
#include <string>
#include <vector>
#include "jvm.h"

int main(int argc, char * argv[])
{
	const char * main_class_name = "Test";
	std::vector<std::string> args;
	args.push_back(main_class_name);
	
	jvm vm;
	vm.load_runtime("rt.jar");
	vm.run(args);

	return 0;
}
