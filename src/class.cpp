#include "class.h"
#include "classloader.h"
#include "memery.h"
#include "jvm.h"

claxx * const_pool::get_class(u2 idx, thread * current_thread)
{
	if (!check(idx)) return nullptr;
	if (cache[idx]) return (claxx*)cache[idx];
	const_pool_item * item = get(idx);
	if (!item || item->tag != CONSTANT_Class) return nullptr;
	class_ref * sym_class = item->sym_class;
	claxx * ret = owner->loader->load_class(sym_class->name->c_str(), current_thread);
	if (ret->state < INIT) ret->loader->initialize_class(ret, current_thread);
	cache[idx] = ret;
	return ret;
}

method * const_pool::get_method(u2 idx, thread * current_thread)
{
	if (!check(idx)) return nullptr;
	if (cache[idx]) return (method*)cache[idx];
	const_pool_item * item = get(idx);
	if (!item || (item->tag != CONSTANT_MethodRef && item->tag != CONSTANT_InterfaceMethodRef)) return nullptr;
	
	method_ref * sym_method = item->sym_method;
	claxx * method_owner = get_class(sym_method->class_index,current_thread);
	if (!method_owner) {
		printf("class not found\n");
		return nullptr;
	}
	method * ret = method_owner->lookup_method(sym_method->name->c_str(), sym_method->discriptor->c_str());
	if (!ret) ret = method_owner->super_class->lookup_method(sym_method->name->c_str(), sym_method->discriptor->c_str());
	if (!ret) {
		for (claxx * interface : method_owner->interfaces) {
			printf("find in %s\n", interface->name->c_str());
			ret = interface->lookup_method(sym_method->name->c_str(), sym_method->discriptor->c_str());
			if (ret) break;
		}
	}
	if (!ret) abort();
	cache[idx] = ret;
	return ret;
}

field * const_pool::get_field(u2 idx, thread * current_thread)
{
	if (!check(idx)) return nullptr;
	if (cache[idx]) return (field*)cache[idx];
	const_pool_item * item = get(idx);
	if (!item || item->tag != CONSTANT_FieldRef) return nullptr;
	
	field_ref * sym_field = item->sym_field;
	claxx * field_owner = get_class(sym_field->class_index, current_thread);
	if (!field_owner) return nullptr;
	field * ret = field_owner->lookup_field(sym_field->name->c_str());
	if (!ret) ret = field_owner->super_class->lookup_field(sym_field->name->c_str());
	cache[idx] = ret;
	return ret;
}

jreference const_pool::get_string(u2 idx, thread * current_thread)
{
	if (!check(idx)) return 0;
	if (cache[idx]) {
		return *(jreference*)&cache[idx];
	}
	const_pool_item * item = get(idx);
	if (!item || item->tag != CONSTANT_String) return 0;
	claxx * string = owner->loader->load_class("java/lang/String", current_thread);
	method * string_init = string->lookup_method("<init>","()V");
	jreference ret = memery::alloc_heap_object(string);
	current_thread->push_frame(string_init);
	*((jreference*)&cache[idx]) = ret;
	return ret;
}


method * claxx::get_init_method()
{
	return lookup_method("<init>", "()V");
}

method * claxx::get_clinit_method()
{
	return lookup_method("<clinit>", "()V");
}

method * claxx::lookup_method(const std::string & name, const std::string & discriptor)
{
	auto byname = methods.find(name);
	if (byname == methods.end()) return nullptr;
	auto bydisc = byname->second.find(discriptor);
	if (bydisc == byname->second.end()) return nullptr;
	return bydisc->second;
}

field *claxx::lookup_field(const std::string & name)
{
	auto res = fields.find(name);
	if (res != fields.end()) return res->second;
	res = static_fields.find(name);
	if (res != fields.end()) return res->second;
	return nullptr;
}

