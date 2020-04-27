
#include "native.h"
#include "jvm.h"
#include "classloader.h"

NATIVE void sun_misc_Unsafe_registerNatives(environment * env,jreference cls)
{
	printf("java_lang_System_registerNatives called %d\n",cls);
}

NATIVE jboolean sun_misc_Unsafe_compareAndSwapObject(environment * env, jreference unsafe, jreference obj, jlong off, jreference expect, jreference newv)
{
	object * oop = memery::ref2oop(obj);
	if (oop->get<jreference>(off) == expect) {
		oop->put<jreference>(off, newv);
		return true;
	}
	return false;
}

NATIVE jlong sun_misc_Unsafe_objectFieldOffset(environment * env, jreference unsafe, jreference f)
{
	return 0;
}

NATIVE jint sun_misc_Unsafe_arrayBaseOffset(environment * env, jreference unsafe,  jreference arr)
{
	printf("array type %d %d\n", unsafe, arr);
	return 0;
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

NATIVE jint sun_misc_Unsafe_getIntVolatile(environment * env, jreference unsafe, jreference obj,  jlong offset)
{
	object * oop = memery::ref2oop(obj);
	return oop->get<jint>(offset);
}

NATIVE jint sun_misc_Unsafe_compareAndSwapInt(environment * env, jreference unsafe, jreference obj, jlong off, jint expect, jint update)
{
	object * oop = memery::ref2oop(obj);
	if (oop->get<jint>(off) == expect) {
		oop->put<jint>(off, update);
		return true;
	}
	return false;
}
