
#include "native.h"
#include "thread.h"
#include "jvm.h"
#include <cstdlib>
#include <thread>
#include <chrono>

NATIVE void java_lang_Thread_registerNatives(environment * env,jreference cls)
{
}

NATIVE jreference java_lang_Thread_currentThread(environment * env, jreference cls) 
{
	return env->get_thread()->mirror;
}

NATIVE jreference java_lang_Thread_isAlive(environment * env, jreference cls) 
{
	return false;
}

NATIVE void java_lang_Thread_start0(environment * env, jreference cls) 
{
	auto t = env->get_vm()->new_thread(cls);
	std::thread(&thread::start, t).detach();
}

NATIVE void java_lang_Thread_setPriority0(environment * env, jreference t) 
{
}

NATIVE void java_lang_Thread_sleep(environment * env, jreference t, jlong time)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
}
