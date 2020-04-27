#include "native.h"

NATIVE void java_lang_Object_registerNatives(environment * env, jreference cls)
{
	printf("java_lang_Object_registerNatives called %d\n", cls);
}

NATIVE int java_lang_Object_hashCode(environment * env, jreference ref)
{
	printf("java_lang_Object_hashCode called return %d\n", ref);
	return ref;
}

NATIVE int java_lang_Object_clone(environment * env, jreference obj)
{
	printf("java_lang_Object_clone called %d\n", obj);
	return  env->clone_object(obj);
}
