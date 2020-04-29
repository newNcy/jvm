
#include "native.h"
#include "jvm.h"
#include "classloader.h"
#include "malloc.h"
#include "log.h"
#include <functional>

NATIVE void sun_misc_Unsafe_registerNatives(environment * env,jreference cls)
{
	printf("java_lang_System_registerNatives called %d\n",cls);
}

NATIVE jboolean sun_misc_Unsafe_compareAndSwapObject(environment * env, jreference unsafe, jreference obj, jlong off, jreference e, jreference x)
{
	object * oop = memery::ref2oop(obj);
	jreference val = oop->get<jreference>(off);
	log::trace("cas %s object this:%d off:%d old:%d == %d new:%d", oop->meta->name->c_str(), obj, off, e, val, x);
	if (val == e) {
		oop->put<jreference>(off, x);
		return true;
	}
	getchar();
	return false;
}

NATIVE jlong sun_misc_Unsafe_objectFieldOffset(environment * env, jreference unsafe, jreference f)
{
	fieldID name_id = env->lookup_field_by_object(f, "name");
	fieldID clazz_id = env->lookup_field_by_object(f, "clazz");
	std::string name = env->get_utf8_string(env->get_object_field(f, name_id));

	claxx * meta = claxx::from_mirror(env->get_object_field(f, clazz_id), env->get_thread());
	log::trace("%s in %s", meta->name->c_str(), name.c_str());
	return meta->lookup_field(name)->offset;
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

NATIVE jlong sun_misc_Unsafe_allocateMemory (environment * env, jreference unsafe, jlong size)
{
	return reinterpret_cast<jlong>(malloc(size));
}

NATIVE void sun_misc_Unsafe_freeMemory (environment * env, jreference unsafe, jlong address)
{
	free(reinterpret_cast<void*>(address));
}

NATIVE void sun_misc_Unsafe_putLong(environment * env, jreference unsafe, jlong address, jlong val)
{
	jlong * pointer = reinterpret_cast<jlong*>(address);
	if (pointer) {
		*pointer = val;
	}
}

NATIVE jbyte sun_misc_Unsafe_getByte(environment * env, jreference unsafe, jlong address)
{
	jbyte * pointer = reinterpret_cast<jbyte*>(address);
	if (pointer) {
		return *pointer & 0xff;
	}
	return 0;
}
