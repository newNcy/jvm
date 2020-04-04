#include "class.h"
#include "classloader.h"
#include "memery.h"
#include "jvm.h"
#include "thread.h"
#include <sstream>

claxx * const_pool::get_class(u2 idx, thread * current_thread)
{
	if (!check(idx)) return nullptr;
	if (cache[idx]) return (claxx*)cache[idx];
	const_pool_item * item = get(idx);
	if (!item || item->tag != CONSTANT_Class) return nullptr;
	class_ref * sym_class = item->sym_class;
	claxx * ret = owner->loader->load_class(sym_class->name->c_str(), current_thread);
	if (ret->state < INITING) ret->loader->initialize_class(ret, current_thread);
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
	if (!ret) ret = field_owner->lookup_static_field(sym_field->name->c_str());
	if (ret->type == T_OBJECT) {
		if (!ret->meta) {
			ret->meta = owner->loader->load_class_from_disc(ret->discriptor->c_str(), current_thread);
		}
	}
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
	jreference ret = current_thread->get_env()->create_string(item->utf8_str->c_str());
	*((jreference*)&cache[idx]) = ret;
	return ret;
}


method * claxx::get_init_method()
{
	return lookup_method("<init>", "()V", false);
}

method * claxx::get_clinit_method()
{
	return lookup_method("<clinit>", "()V", false);
}

method * claxx::lookup_method(const std::string & name, const std::string & discriptor, bool recursive)
{
	auto byname = methods.find(name);
	if (byname != methods.end()) {
		auto bydisc = byname->second.find(discriptor);
		if (bydisc != byname->second.end()) {
			return bydisc->second;
		}
	}
	if (recursive && super_class) {
		return super_class->lookup_method(name, discriptor);
	}
	return nullptr;
}

field * claxx::lookup_field(const std::string & name)
{
	auto res = fields.find(name);
	if (res != fields.end()) return res->second;
	return nullptr;
}
field * claxx::lookup_static_field(const std::string & name)
{
	auto res = static_fields.find(name);
	if (res != static_fields.end()) return res->second;
	return nullptr;
}

claxx * claxx::get_array_claxx(thread * current_thread)
{
	std::stringstream ss;
	ss<<"[L";
	ss<<this->name->c_str()<<";";
	printf("array %s\n", ss.str().c_str());
	return this->loader->load_class(ss.str().c_str(), current_thread);
}

symbol * make_symbol(const std::string & str)
{
	symbol * sym = (symbol*)new char[sizeof(symbol) + str.length()]();
	strcpy((char*)sym->bytes, str.c_str());
	return sym;
}

array_claxx::array_claxx(const std::string & binary_name, classloader * ld, thread * current_thread)
{
	this->name = make_symbol(binary_name);
	this->loader = ld;
	int i = 0;
	while (i < binary_name.length() && binary_name[i++] == '[') dimensions ++;
	this->componen_type = ld->type_of_disc(binary_name[i]);
	if (binary_name[i] == 'L') {
		this->component = ld->load_class(std::string(binary_name, i, binary_name.length()), current_thread);
	}
}
