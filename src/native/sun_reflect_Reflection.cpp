
#include "native.h"
#include "thread.h"

NATIVE jreference sun_reflect_Reflection_getCallerClass(environment * env, jreference cls,  jreference)
{
	return env->get_thread()->current_frame->caller_frame->current_class->mirror;
}

NATIVE jint sun_reflect_Reflection_getClassAccessFlags(environment * env, jreference cls)
{
	return claxx::from_mirror(cls, env->get_thread())->access_flag;
}
