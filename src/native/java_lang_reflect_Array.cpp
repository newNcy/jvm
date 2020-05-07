
#include "native.h"
#include <cstdio>

NATIVE jreference java_lang_reflect_Array_newArray(environment * env, jreference _, jreference component, jint length)
{
	claxx * e = claxx::from_mirror(component, env->get_thread());
	claxx * ec = e->get_array_claxx(env->get_thread());
	return ec->instantiate(length, env->get_thread());
}
