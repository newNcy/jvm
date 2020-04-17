#include "native.h"
#include "class.h"
#include "memery.h"
#include "object.h"
#include "thread.h"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include "jvm.h"
#include "classloader.h"
#include "debug.h"

jreference environment::string_intern(jreference jref)
{
	fieldID f = lookup_field(get_class(jref), "value");
	jreference value = get_object_field(jref, f);
	char buff[1024] = {0};
	for (int i = 0 ; i < array_length(value); i ++) {
		buff[i] = get_array_element(value, i).c;
	}
	return create_string_intern(buff);
}

jreference environment::create_string_intern(const std::string & bytes)
{
	printf("intern %s\n", bytes.c_str());
	fflush(stdout);
	auto ret = interns.find(bytes);
	if (ret != interns.end()) return ret->second;
	jreference ref = create_string(bytes);
	interns[bytes] = ref;
	return ref;
}

jreference environment::create_string(const std::string & bytes)
{
	printf("create string [%s]\n", bytes.c_str());
	function_time t(__PRETTY_FUNCTION__);
	claxx * java_lang_String = get_vm()->get_class_loader()->load_class("java/lang/String", get_thread());
	jreference ref =  memery::alloc_heap_object(java_lang_String);
	jreference value = create_basic_array(T_CHAR, bytes.length());
	int idx = 0;
	for (auto c : bytes) {
		set_array_element(value, idx++, (jchar)c);
	}
	field * fv = java_lang_String->lookup_field("value");
	set_object_field(ref, fv, value);
	return ref;
}

jreference environment::create_basic_array(jtype type, uint32_t length)
{
	std::stringstream ss;
	ss<<"["<<type_disc[type-T_BOOLEAN];
	claxx * meta = get_vm()->get_class_loader()->load_class(ss.str(), get_thread());
	return memery::alloc_heap_array(meta, length);
}

jreference environment::create_obj_array(jreference type, uint32_t length)
{
	auto ec = get_vm()->get_class_loader()->claxx_from_mirror(type);
	auto ac = ec->get_array_claxx(get_thread());
	return memery::alloc_heap_array(ac, length);
}

void environment::set_object_field(jreference ref, fieldID f, jvalue v)
{
	object * oop = memery::ref2oop(ref);
	oop->set_field((field*)f, v);
}

jvalue environment::get_object_field(jreference ref, fieldID f)
{
	object * oop = memery::ref2oop(ref);
	return oop->get_field((field*)f);
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

void environment::set_array_element(jreference ref, jint index, jvalue v)
{
	object * arr = memery::ref2oop(ref);
	arr->set_element(index,v);
}

jvalue environment::get_array_element(jreference ref, jint index)
{
	object * arr = memery::ref2oop(ref);
	return arr->get_element(index);
}

jreference environment::get_class(jreference obj)
{
	object * oop = memery::ref2oop(obj);
	return oop->meta->mirror;
}

fieldID environment::lookup_field(jreference cls, const std::string & name)
{
	claxx * meta = vm->get_class_loader()->claxx_from_mirror(cls);
	printf("lookup field %s in %s\n", name.c_str(), meta->name->c_str());
	return meta->lookup_field(name);
}


methodID environment::lookup_method(jreference cls, const std::string & name, const std::string & discriptor)
{
	claxx * meta = vm->get_class_loader()->claxx_from_mirror(cls);
	return meta->lookup_method(name, discriptor);
}

jvalue environment::callmethod(methodID m, jreference obj)
{
	operand_stack stack(1);
	stack.push(obj);
	return get_thread()->call(static_cast<method*>(m), &stack, false, true);
}
NATIVE int Test_test(environment * env,jreference cls, int arg) 
{
	printf("Tes.test called arg:%d %d\n", cls, arg);
	return arg * 2;
}
