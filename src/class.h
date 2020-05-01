#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <set>
#include "classfile.h"
#include "attribute.h"
#include "util.h"

class memery;
class thread;
class claxx;
class method;
class field;
class classloader;

typedef int8_t jboolean;
typedef uint8_t jbyte;
typedef uint16_t jchar;
typedef int16_t jshort;
typedef int32_t jint;
typedef float jfloat; 
typedef int64_t jlong;
typedef double jdouble;
typedef uint32_t jreference;

enum jtype 
{
	T_VOID,
	T_BOOLEAN = 4,
	T_CHAR,
	T_FLOAT,
	T_DOUBLE,
	T_BYTE,
	T_SHORT,
	T_INT,
	T_LONG, 
	T_OBJECT,
};

union jvalue
{
	jboolean	z;
	jbyte		b;
	jchar		c;
	jshort		s;
	jint		i;
	jfloat		f;
	jlong		j;
	jdouble		d;
	jreference	l;
	jvalue() {}
	jvalue(jboolean v):		z(v) {}
	jvalue(jbyte v):		b(v) {}
	jvalue(jchar v):		c(v) {}
	jvalue(jshort v):		s(v) {}
	jvalue(jint v):			i(v) {}
	jvalue(jfloat v):		f(v) {}
	jvalue(jlong v):		j(v) {}
	jvalue(jdouble v):		d(v) {}
	jvalue(jreference v):	l(v) {}
	operator jboolean()		{ return z; }
	operator jchar()		{ return c; }
	operator jfloat()		{ return f; }
	operator jdouble()		{ return d; }
	operator jbyte()		{ return b; }
	operator jshort()		{ return s; }
	operator jint()			{ return i; }
	operator jlong()		{ return j; }
	operator jreference()	{ return l; }
};


template <typename T> struct element_type_helper;
template <> struct element_type_helper <jboolean>	{ using type = jint; };
template <> struct element_type_helper <jchar>		{ using type = jint; };
template <> struct element_type_helper <jbyte>		{ using type = jint; };
template <> struct element_type_helper <jshort>		{ using type = jint; };
template <> struct element_type_helper <jint>		{ using type = jint; };
template <> struct element_type_helper <jlong>		{ using type = jlong; };
template <> struct element_type_helper <jfloat>		{ using type = jfloat; };
template <> struct element_type_helper <jdouble>	{ using type = jdouble; };
template <> struct element_type_helper <jreference>	{ using type = jreference; };
template <typename RawType>
using element_type = typename element_type_helper<RawType>::type;



template <typename T> struct jtype_value_traits		{ enum { type_value = T_VOID};};

template <> struct jtype_value_traits<jboolean>		{ enum { type_value = T_BOOLEAN};};
template <> struct jtype_value_traits<jchar>		{ enum { type_value = T_CHAR};};	
template <> struct jtype_value_traits<jfloat>		{ enum { type_value = T_FLOAT};};
template <> struct jtype_value_traits<jdouble>		{ enum { type_value = T_DOUBLE};};
template <> struct jtype_value_traits<jbyte>		{ enum { type_value = T_BYTE};};
template <> struct jtype_value_traits<jshort>		{ enum { type_value = T_SHORT};};
template <> struct jtype_value_traits<jint>			{ enum { type_value = T_INT};};
template <> struct jtype_value_traits<jlong>		{ enum { type_value = T_LONG};};
template <> struct jtype_value_traits<jreference>	{ enum { type_value = T_OBJECT};};

const static uint32_t type_size[] = {0,0,0,0, 1, 2, 4, 8, 1, 2, 4, 8, 4};


const static char type_disc[] = {'V',0,0,0 ,'Z', 'C', 'F', 'D', 'B', 'S', 'I', 'J', /* x */'@'};
const static char * type_text[] = {"void","x","x","x","boolean", "char", "float", "double", "byte", "short", "int", "long", "objet"};

struct symbol_ref 
{
};

struct class_ref : public symbol_ref
{
	symbol * name = nullptr;
};

struct field_ref : public class_ref
{
	u2 class_index;
	symbol * discriptor = nullptr;
};
struct method_ref : public field_ref {};
struct interface_method_ref : public field_ref {};

struct method_handle_ref : public symbol_ref 
{
	u1 reference_kind = 0;
	union 
	{
		field_ref * sym_field = nullptr;
		method_ref * sym_method;
		interface_method_ref * sym_interface_method;
	};
};

struct method_type_ref : public symbol_ref
{
	symbol * discriptor = nullptr;
};

struct invoke_dynamic_ref : public method_ref 
{
	u2 boostrap_method_attr_index;
};

struct const_pool_item 
{ 
	u1 tag = 0;
	union 
	{
		class_ref *				sym_class				= nullptr;
		field_ref *				sym_field				;
		method_ref *			sym_method				;
		interface_method_ref *	sym_interface_method	;
		method_handle_ref *		sym_method_handle		;
		method_type_ref *		sym_method_type			;
		invoke_dynamic_ref *	sym_invoke_dynamic		;
		symbol *				utf8_str				;
	};
	jvalue value;
};


struct meta_base : public attribute_holder
{
	u2 access_flag;
	bool is_public() const			{ return access_flag & 0x0001; }
	bool is_private() const			{ return access_flag & 0x0002; }
	bool is_protect() const			{ return access_flag & 0x0004; }
	bool is_static() const			{ return access_flag & 0x0008; }
	bool is_final() const			{ return access_flag & 0x0010; }
	bool is_synchronized() const	{ return access_flag & 0x0020; }
	bool is_bridge() const			{ return access_flag & 0x0040; }
	bool is_varargs() const			{ return access_flag & 0x0080; }
	bool is_native() const			{ return access_flag & 0x0100; }
	bool is_interface() const		{ return access_flag & 0x0200; }
	bool is_abstract() const		{ return access_flag & 0x0400; }
	bool is_strict() const			{ return access_flag & 0x0800; }
	bool is_synthetic() const		{ return access_flag & 0x1000; }

	virtual bool is_class() { return false; }
	virtual bool is_method() { return false; }
	virtual bool is_field() { return false; }
};

struct name_and_type_meta : public meta_base
{
	symbol * name = nullptr;
	symbol * discriptor = nullptr;
	claxx * owner = nullptr;
};

struct field : public name_and_type_meta 
{
	bool is_field() { return true; }
	int offset = -1;
	jtype type;
	claxx * meta = nullptr; 
	claxx * get_meta(thread * current_thread);
};
struct method : public name_and_type_meta 
{
	bool is_method() { return true; }
	jint slot = 0;
	std::vector<jtype> arg_types; //存储类型
	std::vector<symbol*> param_types; //具体类型
	uint32_t arg_space = 0;
	jtype  ret_type;
	void (*native_method)() = nullptr;
};

struct const_pool
{
	claxx * owner = nullptr;
	std::vector<const_pool_item *> data;
	std::vector<void*> cache;
	public:
	size_t count = 0;
	const_pool(size_t size): data(size), cache(size) ,count(size)
	{
		for (auto p : cache) p = nullptr;
	}

	bool check(int idx) { return idx >= 0 && idx < data.capacity(); }

	void put(const_pool_item * item, int idx)
	{
		if (!check(idx)) return;
		data[idx] = item;
	}
	const_pool_item * get(int idx)
	{
		if (!check(idx)) return nullptr;
		return data[idx];
	}	
	u1 tag_at(int idx)
	{
		if (!check(idx)) return 0;
		return data[idx]->tag;
	}
	//运行时取
	claxx * get_class(u2 idx, thread * current_thread);
	method * get_method(u2 idx, thread * current_thread);
	field * get_field(u2 idx, thread * current_thread);
	jreference get_string(u2 idx, thread * current_thread);

	void resolution();
};


enum class_state
{
	CREATE,
	LOAD,
	LINKING,
	LINKED,
	INITING,
	INITED
};

struct array_claxx;

struct claxx : public class_ref, meta_base 
{
	class_state state = CREATE;
	claxx * super_class = nullptr;
	classloader * loader = nullptr;
	std::vector<claxx*> interfaces;
	std::map<std::string,std::map<std::string, method *> > methods;
	std::vector<method*> method_by_index;
	std::map<std::string,field*> fields;
	std::map<std::string,field*> static_fields;
	std::set<claxx * > childs;
	const_pool * cpool = nullptr;

	size_t static_member_size = 0;
	size_t member_size = 0;
	jreference mirror = 0;
	jreference static_obj = 0; //暂时这么存放静态数据

	virtual bool is_class() { return !is_interface(); }
	virtual bool is_array() { return false; }
	virtual bool is_primitive() { return false; }
	size_t static_size() { return static_member_size; }
	size_t size() const { return member_size;}
	virtual size_t size(int length) const { return 0;};
	method * get_init_method();
	method * get_clinit_method();
	method * lookup_method(const std::string & name, const std::string & discriptor, bool recursive = true);
	method * method_at(jint slot) { return method_by_index[slot]; }
	field * lookup_field(const std::string & name);
	field * lookup_static_field(const std::string & name);
	claxx * get_array_claxx(thread *);
	bool subclass_of(claxx * T);
	bool has_implement(claxx * T);
	bool check_cast_to(claxx * T);
	jreference instantiate(thread *);
	
	virtual jreference instantiate(int length, thread *) { throw "Not Array Type";}
	std::vector<const method *> constructors();
	static claxx * from_mirror(jreference cls, thread *);
};

struct array_claxx : public claxx
{
	virtual bool is_class() override { return false; }
	virtual bool is_array() override { return true; }
	jreference instantiate(int length, thread *) override ;
	size_t size(int length) const override { return length*type_size[componen_type];}
	uint32_t dimensions = 0;
	array_claxx(const std::string & binary_name, classloader * ld, thread * current_thread);
	jtype componen_type;
	claxx * component = nullptr;
};

struct primitive_claxx : public claxx 
{
	virtual bool is_primitive() { return true; }
	primitive_claxx(jtype t, classloader *ld);
	jtype type;
};

symbol * make_symbol(const std::string & str);
