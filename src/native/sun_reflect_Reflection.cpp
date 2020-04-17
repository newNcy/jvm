
#include "native.h"
#include "thread.h"

NATIVE jreference sun_reflect_Reflection_getCallerClass(environment * env, jreference cls,  jreference)
{
	return env->get_thread()->current_frame->caller_frame->current_class->mirror;
}


