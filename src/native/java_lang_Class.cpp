
#include "native.h"
#include "jvm.h"
#include "classloader.h"
#include <cstdio>

NATIVE int java_lang_Class_desiredAssertionStatus0(environment * env, jreference ref) 
{
	return 1;
}
NATIVE void java_lang_Class_registerNatives(environment * env,jreference cls)
{
	printf("java_lang_Object_registerNatives called %d\n", cls);
}

NATIVE jreference java_lang_Class_getPrimitiveClass(environment * env, jreference cls,  jreference ref)
{
	printf("java_lang_Class_getPrimitiveClass called %d prop ref %d\n", cls, ref);
	return 128;
}

NATIVE jreference java_lang_Class_getDeclaredFields0(environment * env, jreference cls, jboolean z)
{
	auto ec = env->get_vm()->get_class_loader()->load_class("java/lang/reflect/Field", env->get_thread());
	auto cls_meta = env->get_vm()->get_class_loader()->claxx_from_mirror(cls);
	jreference ret = env->create_obj_array(ec->mirror, cls_meta->fields.size());
	auto name = env->lookup_field(ec->mirror, "name");
	auto type = env->lookup_field(ec->mirror, "type");
	int i = 0;
	for (auto f : cls_meta->fields) {
		jreference e = memery::alloc_heap_object(ec);
		jreference nv = env->create_string_intern(f.second->name->c_str());
		env->set_object_field(e, name, nv);
		auto fm = f.second->get_meta(env->get_thread())->mirror;
		//env->set_object_field(e, type, );
		env->set_array_element(ret, i++, e);
	}
	return ret;
}


NATIVE jreference java_lang_Class_getName0(environment * env, jreference cls)
{
	claxx * meta = env->get_vm()->get_class_loader()->claxx_from_mirror(cls);
	return env->create_string_intern(meta->name->c_str());
}

NATIVE jreference java_lang_Class_forName0(environment * env, jreference cls, jreference name)
{
	fieldID value = env->lookup_field(env->get_class(name), "value");
	jreference char_array = env->get_object_field(name, value);
	char buff [128] = {0};
	for (int i = 0 ; i < env->array_length(char_array); i ++) {
		buff[i] = env->get_array_element(char_array, i).c;
	}
	return env->get_vm()->get_class_loader()->load_class(buff, env->get_thread())->mirror;
}
