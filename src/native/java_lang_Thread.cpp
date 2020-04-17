
#include "native.h"
#include "thread.h"

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
}
