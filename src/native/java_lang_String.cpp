#include "native.h"
#include <map>


NATIVE jreference java_lang_String_intern(environment * env, jreference obj)
{
	return env->string_intern(obj);
}
