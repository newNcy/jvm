#pragma once
#include "class.h"
#include "memery.h"
#include <vector>
#include <string>
#include <cstdio>
#include <stdint.h>
#include "thread.h"

class jvm
{
	classloader * cloader = nullptr;
	std::vector<thread *> threads;
	public:
	thread * new_thread();
	jvm();
	~jvm();
	void load_runtime(const std::string & runtime_path);
	void run(std::vector<std::string> args);
};
