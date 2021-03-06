#include "native.h"
#include "log.h"

NATIVE void java_lang_Object_registerNatives(environment * env, jreference cls)
{
	log::debug("java_lang_Object_registerNatives called %d\n", cls);
}

NATIVE int java_lang_Object_hashCode(environment * env, jreference ref)
{
	log::debug("java_lang_Object_hashCode called return %d\n", ref);
	return ref;
}

NATIVE int java_lang_Object_clone(environment * env, jreference obj)
{
	log::debug("java_lang_Object_clone called %d\n", obj);
	return  env->clone_object(obj);
}

NATIVE jreference java_lang_Object_getClass(environment * env, jreference obj)
{
	return  env->get_class(obj);
}

NATIVE void java_lang_Object_notifyAll(environment * env, jreference obj)
{
}

NATIVE void java_lang_Object_wait(environment * env, jreference obj)
{
}
