
#include "native.h"
#include "thread.h"
#include "jvm.h"
#include <cstdlib>

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
	auto m = env->lookup_method_by_object(cls, "run", "()V");
	auto t = env->get_vm()->new_thread(cls);
	t->get_env()->callmethod(m, cls);
}

NATIVE void java_lang_Thread_setPriority0(environment * env, jreference t) 
{
}
