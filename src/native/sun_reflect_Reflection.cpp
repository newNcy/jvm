
#include "native.h"
#include "thread.h"

NATIVE jreference sun_reflect_Reflection_getCallerClass(environment * env, jreference cls,  jreference)
{
	return env->get_thread()->current_frame->caller_frame->caller_frame->current_class->mirror;
}

NATIVE jint sun_reflect_Reflection_getClassAccessFlags(environment * env, jreference cls)
{
    /** Retrieves the access flags written to the class file. For
        inner classes these flags may differ from those returned by
        Class.getModifiers(), which searches the InnerClasses
        attribute to find the source-level access flags. This is used
        instead of Class.getModifiers() for run-time access checks due
        to compatibility reasons; see 4471811. Only the values of the
        low 13 bits (i.e., a mask of 0x1FFF) are guaranteed to be
        valid. */
	return claxx::from_mirror(cls, env->get_thread())->access_flag & 0x1fff;
}
