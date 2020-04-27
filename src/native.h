#pragma once



#include "class.h"
#include <map>
#include <string>
#include <utility>
#include "frame.h"
class jvm;

using fieldID = void*;
using methodID = void*;


class environment
{ 
	friend class jvm;
	private:
		jvm * vm = nullptr;
		thread * current_thread = nullptr;
		std::map<std::string, jreference> interns;
		jvalue callmethod(methodID m, array_stack & args);
	public:
		environment(jvm * this_vm, thread * this_thread):vm(this_vm), current_thread(this_thread) {}
		jvm * get_vm() { return vm; }
		thread * get_thread() { return current_thread; }
		
		jreference create_string(const std::string & bytes);
		jreference create_string_intern(const std::string & bytes);
		jreference string_intern(jreference jref);
		std::string get_utf8_string(jreference ref);

		jint array_length(jreference ref);
		void set_array_element(jreference ref, jint index, jvalue v);
		jvalue get_array_element(jreference ref, jint index);
		void set_object_field(jreference ref, fieldID f, jvalue v);

		jvalue get_object_field(jreference ref, fieldID  f);

		jreference create_basic_array(jtype type, uint32_t length);
		jreference create_obj_array(jreference type, uint32_t length);
		jreference get_class(jreference obj);

		fieldID lookup_field_by_object(jreference cls, const std::string & name);
		fieldID lookup_field_by_class(jreference cls, const std::string & name);
		methodID lookup_method_by_object(jreference cls, const std::string & name, const std::string & dis);
		methodID lookup_method_by_class(jreference cls, const std::string & name, const std::string & dis);
		jreference clone_object(jreference obj);
		jreference lookup_class(const std::string & name);

		template <typename ... Args>
		jvalue callmethod(methodID m, jreference class_or_this, Args ... args);

		void dumpobj(jreference obj);
};


template <typename ... Args>
jvalue environment::callmethod(methodID m, jreference class_or_this, Args ... args)
{
	auto mh = static_cast<method*>(m);
	int args_size = 0;
	for (auto arg : mh->arg_types) {
		if (type_size[arg] > sizeof(int)) args_size ++;
		args_size ++;
	}

	array_stack arg_pack(args_size+1);
	arg_pack.push(class_or_this, args...);
	return callmethod(m, arg_pack);
}


#define NATIVE extern "C"
