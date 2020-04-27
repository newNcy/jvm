#include "native.h"
#include "class.h"
#include "frame.h"
#include "memery.h"
#include "object.h"
#include "thread.h"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include "jvm.h"
#include "classloader.h"
#include "debug.h"

jreference environment::string_intern(jreference jref)
{
	fieldID f = lookup_field_by_class(get_class(jref), "value");
	jreference value = get_object_field(jref, f);
	char buff[1024] = {0};
	for (int i = 0 ; i < array_length(value); i ++) {
		buff[i] = get_array_element(value, i).c;
	}
	return create_string_intern(buff);
}

jreference environment::create_string_intern(const std::string & bytes)
{
	//printf("intern %s\n", bytes.c_str());
	auto ret = interns.find(bytes);
	if (ret != interns.end()) return ret->second;
	jreference ref = create_string(bytes);
	interns[bytes] = ref;
	return ref;
}

int unicode_length(const char * buf, int len)
{
	int count = 0;
	for (int i = 0 ; i < len; i++) {
		count ++;
		unsigned char c = buf[i];
		if (c == 0xed) {
			i += 5;
		}else if (c >= 0xe0) { // 1110 0000
			i += 2;
		}else if (c >= 0xc0) { // 1100 0000
			i += 1;
		}
	}
	return count;
}
	
std::string environment::get_utf8_string(jreference ref)
{
	fieldID valueid = lookup_field_by_class(get_class(ref), "value");
	jreference value = get_object_field(ref, valueid);
	if (!value) return "";
	std::vector<char> bytes;
	int len = array_length(value);
	for (int i = 0; i < len; i++) {
		char c = get_array_element(value, i).c;
		bytes.push_back(c);
	}
	bytes.push_back(0);
	return std::string(&bytes[0]);
}

jchar utf8_to_unicode(const char * buf, int len, int & took)
{
		unsigned char c = *buf;
		jchar cur = 0;
		if (c == 0xed) {
			cur = 0x10000 + ((buf[1] & 0x0f) << 16) + ((buf[3] & 0x3f) << 10) + ((buf[4] & 0x0f) << 6) + (buf[5] & 0x3f);
			took = 6;
		}else if (c >= 0xe0) { // 1110 0000
			cur = ((buf[0] & 0xf) << 12) + ((buf[1] & 0x3f) << 6) + (buf[2] & 0x3f);
			took = 3;
		}else if (c >= 0xc0) { // 1100 0000
			cur = ((buf[0] & 0x1f) << 6) + (buf[1] & 0x3f);
			took = 2;
		}else {
			cur = buf[0] & 0x7f;
			took = 1;
		}
		return cur;
}

jreference environment::create_string(const std::string & bytes)
{
	//printf("create string [%ld, %s]\n", bytes.length(), bytes.c_str());
	int length = unicode_length(bytes.c_str(), bytes.length());

	function_time t(__PRETTY_FUNCTION__);
	claxx * java_lang_String = get_vm()->get_class_loader()->load_class("java/lang/String", get_thread());
	jreference ref =  memery::alloc_heap_object(java_lang_String);
	jreference value = create_basic_array(T_CHAR, length);
	int idx = 0;
	int took = 0;
	while (took < bytes.length()) {
		int t = 0;
		set_array_element(value, idx++, utf8_to_unicode(bytes.c_str() + took, bytes.length(), t));
		took += t;
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
	auto rf = static_cast<field*>(f);
	oop->set_field((field*)f, v);
}

jvalue environment::get_object_field(jreference ref, fieldID f)
{
	object * oop = memery::ref2oop(ref);
	return oop->get_field((field*)f);
}

jint environment::array_length(jreference ref)
{
	if (!ref) {
		throw "java/lang/NullPointerException";
	}
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
	if (index >= arr->array_length()) {
		throw "java/lang/ArrayIndexOutOfBoundsException";
	}
	return arr->get_element(index);
}

jreference environment::get_class(jreference obj)
{
	object * oop = memery::ref2oop(obj);
	return oop->meta->mirror;
}


jreference environment::lookup_class(const std::string & name)
{
	claxx * c = get_vm()->get_class_loader()->load_class(name, get_thread());
	if (c) return c->mirror;
	return null;
}

fieldID environment::lookup_field_by_class(jreference cls, const std::string & name)
{
	claxx * meta = vm->get_class_loader()->claxx_from_mirror(cls);
	return meta->lookup_field(name);
}

jreference environment::clone_object(jreference obj)
{
	if (!obj) return null;
	object * oop = memery::ref2oop(obj);
	if (!oop) return null;

	jreference ret = null;
	if (oop->is_array()) {
		int length = oop->array_length();
		ret = oop->meta->instantiate(length, get_thread());
		for (int i = 0 ; i < length; i++) {
			set_array_element(ret, i, get_array_element(obj, i));
		}
	}else {
		ret = oop->meta->instantiate(get_thread());
		for (auto f : oop->meta->fields) {
			set_object_field(ret, f.second, get_object_field(obj, f.second));
		}
	}
	return ret;
}

fieldID environment::lookup_field_by_object(jreference obj, const std::string & name) 
{
	return lookup_field_by_class(get_class(obj), name);
}

methodID environment::lookup_method_by_class(jreference cls, const std::string & name, const std::string & discriptor)
{
	claxx * meta = vm->get_class_loader()->claxx_from_mirror(cls);
	return meta->lookup_method(name, discriptor);
}
methodID environment::lookup_method_by_object(jreference obj, const std::string & name, const std::string & discriptor) 
{
	return lookup_method_by_class(get_class(obj), name, discriptor);
}

jvalue environment::callmethod(methodID m, array_stack & args)
{
	auto mh = static_cast<method*>(m);
	return get_thread()->call(mh, &args, false, true);
}

NATIVE int Test_test(environment * env,jreference cls, int arg) 
{
	printf("Tes.test called arg:%d %d\n", cls, arg);
	return arg * 2;
}
		
void environment::dumpobj(jreference obj)
{

}
