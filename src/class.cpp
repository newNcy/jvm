#include "class.h"
#include "classfile.h"
#include "classloader.h"
#include "memery.h"
#include "jvm.h"
#include "thread.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include "log.h"


claxx * const_pool::get_class(u2 idx, thread * current_thread)
{
	if (!check(idx)) return nullptr;
	if (cache[idx]) return (claxx*)cache[idx];
	const_pool_item * item = get(idx);
	if (!item || item->tag != CONSTANT_Class) return nullptr;
	class_ref * sym_class = item->sym_class;
	claxx * ret = owner->loader->load_class(sym_class->name->c_str(), current_thread);
	//if (ret->state < INITED) ret->loader->initialize_class(ret, current_thread);
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
		return nullptr;
	}
	method * ret = method_owner->lookup_method(sym_method->name->c_str(), sym_method->discriptor->c_str());
	if (!ret) ret = method_owner->super_class->lookup_method(sym_method->name->c_str(), sym_method->discriptor->c_str());
	if (!ret) {
		for (claxx * interface : method_owner->interfaces) {
			ret = interface->lookup_method(sym_method->name->c_str(), sym_method->discriptor->c_str());
			if (ret) break;
		}
	}
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
	if (!ret) {
		throw std::runtime_error(std::string("field:") + field_owner->name->c_str() + " " + sym_field->name->c_str());
	}
	if (!ret->meta) {
		if (ret->type == T_OBJECT) {
			ret->meta = owner->loader->load_class_from_disc(ret->discriptor->c_str(), current_thread);
		}else {
			ret->meta = owner->loader->load_class(ret->discriptor->c_str(), current_thread);
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
	jreference ret = current_thread->get_env()->create_string_intern(item->utf8_str->c_str());
	*((jreference*)&cache[idx]) = ret;
	return ret;
}

claxx * field::get_meta(thread * current_thread)
{
	if (meta) return meta;
	if (type == T_OBJECT && discriptor->at(0) != '[') {
		meta = owner->loader->load_class_from_disc(discriptor->c_str(), current_thread);
		owner->loader->initialize_class(meta, current_thread);
	}else {
		meta = owner->loader->load_class(discriptor->c_str(), current_thread);
	}
	if (!meta) {
		throw std::runtime_error(std::string("ClassNotFound") + discriptor->c_str());
	}
	return meta;
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
	//printf("look up %s %s in %s\n", name.c_str(), discriptor.c_str(), this->name->c_str());
	auto byname = methods.find(name);
	if (byname != methods.end()) {
		auto bydisc = byname->second.find(discriptor);
		if (bydisc != byname->second.end()) {
			return bydisc->second;
		}
	}
	/*
	 for (auto itf : interfaces) {
		auto m = itf->lookup_method(name, discriptor, recursive);
		if (m) return m;
	}
	*/

	if (recursive && super_class) {
		return super_class->lookup_method(name, discriptor);
	}

	return nullptr;
}

claxx * claxx::from_mirror(jreference cls, thread * current)
{
	return current->get_env()->get_vm()->get_class_loader()->claxx_from_mirror(cls);
}
std::vector<const method *> claxx::constructors()
{
	std::vector<const method *> ret;
	auto constructors = methods.find("<init>");
	if (constructors != methods.end()) {
		for (auto m : constructors->second) {
			ret.push_back(m.second);
		}
	}
	return ret;
}

field * claxx::lookup_field(const std::string & name)
{
	//printf("lookup %s in %s\n",name.c_str(), this->name->c_str());
	for (auto f : fields) {
		//printf("%s %s : %p\n", f.first.c_str(), f.second->discriptor->c_str(), f.second);
	}
	auto res = fields.find(name);
	if (res != fields.end()) return res->second;
	
	res = static_fields.find(name);
	if (res != static_fields.end()) return res->second;
	return nullptr;
}
field * claxx::lookup_static_field(const std::string & name)
{
	auto res = static_fields.find(name);
	if (res != static_fields.end()) return res->second;
	return nullptr;
}

bool claxx::has_implement(claxx * T)
{
	for (auto inter : this->interfaces) {
		if (inter == T || inter->subclass_of(T)) return true;
	}
	return false;
}

bool claxx::subclass_of(claxx * T)
{
	if (!T) return false;
	if (this->super_class) {
		if (super_class == T || super_class->subclass_of(T)) return true;
	}
	for (auto inter : this->interfaces) {
		if (inter == T || inter->subclass_of(T)) return true;
	}

	return false;
}
bool claxx::check_cast_to(claxx * T)
{
	log::debug("check cast %s to %s", this->name->c_str(), T->name->c_str());
	if (!T) return false;
	if (this->is_class()) { //1.
		if (T->is_class()) {
			if (this == T || this->subclass_of(T)) return true; 
		}else if (T->is_interface()) {
			return this->subclass_of(T);
		}
	}else if (this->is_interface()) {
		if (T->is_class()) return T->name->equals("java/lang/Object");
		else if (T->is_interface())  return this == T || this->subclass_of(T);
	}else if (this->is_array()) {
		auto SC = static_cast<array_claxx*>(this)->component;
		if (T->is_class()) return T->name->equals("java/lang/Object");
		else if (T->is_interface()) throw "un implements";
		else if (T->is_array()) {
			auto TC = static_cast<array_claxx*>(this)->component;
			if (SC->is_primitive() && TC->is_primitive()) return SC == TC;
			else {
				return SC->check_cast_to(TC);
			}
		}
	}
	return false;
}
claxx * claxx::get_array_claxx(thread * current_thread)
{
	std::stringstream ss;
	ss<<"[L";
	ss<<this->name->c_str()<<";";
	//printf("array %s\n", ss.str().c_str());
	return this->loader->load_class(ss.str().c_str(), current_thread);
}

jreference claxx::instantiate(thread * current_thread)
{
	if (this->state < INITED) loader->initialize_class(this, current_thread);
	return memery::alloc_heap_object(this);
}

jreference array_claxx::instantiate(int length, thread * current_thread)
{
	if (this->state < INITED) loader->initialize_class(this, current_thread);
	return memery::alloc_heap_array(this, length);
}

array_claxx::array_claxx(const std::string & binary_name, classloader * ld, thread * current_thread)
{
	this->name = symbol_pool::instance().put(binary_name);
	this->loader = ld;
	int i = 0;
	while (i < binary_name.length() && binary_name[i] == '[') i++,dimensions ++;
	this->componen_type = ld->type_of_disc(binary_name[i]);
	this->access_flag = 0;
	if (binary_name[i] == 'L') {
		std::string componont_name(binary_name, i);
		this->component = ld->load_class_from_disc(componont_name, current_thread);
	}else {
		this->component = ld->load_class(&binary_name[i], current_thread);
	}
	super_class = ld->load_class("java/lang/Object", current_thread);
	ld->create_mirror(this, current_thread);
}

primitive_claxx::primitive_claxx(jtype t, classloader *ld):type(t)
{
	this->loader = ld;
	this->name = symbol_pool::instance().put(std::string("") + type_disc[t]);
	ld->record_claxx(this);
	ld->create_mirror(this, nullptr);
	//printf("primitive %s ref %d\n", this->name->c_str(), this->mirror);
}
