
#include "native.h"
#include "jvm.h"
#include "classloader.h"

NATIVE void sun_misc_Unsafe_registerNatives(environment * env,jreference cls)
{
	printf("java_lang_System_registerNatives called %d\n",cls);
}

NATIVE jboolean sun_misc_Unsafe_compareAndSwapObject(environment * env, jreference unsafe, jreference old , jlong off, jreference expect, jreference newv)
{
	if (old == expect) {
	}
	return true;
}
NATIVE jlong sun_misc_Unsafe_objectFieldOffset(environment * env, jreference unsafe, jreference f)
{
	return sizeof(object);
}

NATIVE jint sun_misc_Unsafe_arrayBaseOffset(environment * env, jreference cls,  jreference)
{
	return sizeof(object);
}

NATIVE jint sun_misc_Unsafe_arrayIndexScale(environment * env, jreference unsafe ,  jreference cls)
{
	claxx * ac = env->get_vm()->get_class_loader()->claxx_from_mirror(cls);
	return type_size[dynamic_cast<array_claxx*>(ac)->componen_type];
}


NATIVE jint sun_misc_Unsafe_addressSize(environment * env, jreference cls,  jreference)
{
	return sizeof(jreference);
}
