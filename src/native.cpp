#include "native.h"
#include "class.h"
#include "memery.h"
#include "object.h"
#include "thread.h"
#include <cstdio>
#include <sstream>
#include "jvm.h"
#include "classloader.h"
#include "debug.h"

jreference environment::create_string(const std::string & bytes)
{
	function_time t(__PRETTY_FUNCTION__);
	claxx * java_lang_String = get_vm()->get_class_loader()->load_class("java/lang/String", get_thread());
	jreference ref =  memery::alloc_heap_object(java_lang_String);
	return ref;
}

jreference environment::create_basic_array(jtype type, uint32_t length)
{
	std::stringstream ss;
	ss<<"["<<type_disc[type-T_BOOLEAN];
	claxx * meta = get_vm()->get_class_loader()->load_class(ss.str(), get_thread());
	return memery::alloc_heap_array(meta, length);
}

jvalue environment::get_object_field(jreference ref, field * f)
{
	object * oop = memery::ref2oop(ref);
	return oop->get_field(f);
}
jint environment::array_length(jreference ref)
{
	object * oop = memery::ref2oop(ref);
	if (!oop->is_array()) {
		printf("not an array\n");
		return 0;
	}
	return oop->array_length();
}

NATIVE void java_lang_Object_registerNatives(environment * env)
{
	printf("java_lang_Object_registerNatives called\n");
}

NATIVE int java_lang_Object_hashCode(environment * env, jreference ref)
{
	printf("java_lang_Object_hashCode called return %d\n", ref);
	return ref;
}

NATIVE void java_lang_Class_registerNatives(environment * env)
{
	printf("java_lang_Object_registerNatives called\n");
}

NATIVE void java_lang_System_registerNatives(environment * env, jreference prop)
{
	printf("java_lang_System_registerNatives called prop:%d\n", prop);
}

NATIVE void java_lang_System_loadLibrary(environment * env)
{
	printf("java_lang_System_loadLibrarycalled\n");
}
NATIVE void java_lang_System_initProperties(environment * env, jreference prop)
{
	printf("java_lang_System_initProperties called prop ref %d\n", prop);
}

NATIVE int Test_test(environment * env, int arg) 
{
	printf("Tes.test called arg:%d\n", arg);
	return arg * 2;
}
