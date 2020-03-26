#pragma once
#include <string>
#include <vector>
#include <map>
#include "classfile.h"
#include "attribute.h"

class memery;
class thread;
class claxx;
class method;
class field;
class classloader;
typedef cp_utf8 symbol;

typedef uint8_t jboolean;
typedef int8_t jbyte;
typedef uint16_t jchar;
typedef int16_t jshort;
typedef int32_t jint;
typedef float jfloat; 
typedef int64_t jlong;
typedef double jdouble;
typedef uint32_t jreference;

enum jtype 
{
	VOID,
	BOOLEAN = 4,
	CHAR,
	FLOAT,
	DOUBLE,
	BYTE,
	SHORT,
	INT,
	LONG, 
	OBJECT,
};

union jvalue
{
	jboolean	as_boolean;
	jbyte		as_byte;
	jchar		as_char;
	jshort		as_short;
	jint		as_int;
	jfloat		as_float;
	jlong		as_long;
	jdouble		as_double;
	jreference	as_reference;
};



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
	symbol * name = nullptr;
	symbol * discriptor = nullptr;
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
		jvalue value;
	};
};


struct meta_base
{
	u2 access_flag;
	std::map<std::string, attribute * > attributes;
	bool is_public() const			{ return access_flag & 0x0001; }
	bool is_private() const			{ return access_flag & 0x0002; }
	bool is_protect() const			{ return access_flag & 0x0004; }
	bool is_static() const			{ return access_flag & 0x0008; }
	bool is_final() const			{ return access_flag & 0x0010; }
	bool is_synchronized() const	{ return access_flag & 0x0020; }
	bool is_bridge() const			{ return access_flag & 0x0040; }
	bool is_varargs() const			{ return access_flag & 0x0080; }
	bool is_native() const			{ return access_flag & 0x0100; }
	bool is_abstract() const		{ return access_flag & 0x0200; }
	bool is_strict() const			{ return access_flag & 0x0400; }
	bool is_synthetic() const		{ return access_flag & 0x0800; }

	virtual bool is_class() { return false; }
	virtual bool is_array() { return false; }
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
};
struct method : public name_and_type_meta 
{
	bool is_method() { return true; }
	std::vector<jtype> arg_types;
	jtype  ret_type;
};

struct const_pool
{
	claxx * owner = nullptr;
	std::vector<const_pool_item *> data;
	std::vector<void*> cache;
	public:
	const_pool(size_t size): data(size), cache(size) 
	{
		for (auto p : cache) p = nullptr;
	}

	bool check(int idx) { return idx >= 0 && idx < data.capacity(); }

	void put(const_pool_item * item, int idx)
	{
		if (!check(idx)) return;
		data[idx] = item;
	}
	void * pre_get(int idx, tag_enum tag);
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
	LINK,
	INIT
};


struct claxx : public class_ref, meta_base 
{
	class_state state = CREATE;
	claxx * super_class;
	classloader * loader = nullptr;
	std::vector<claxx*> interfaces;
	std::map<std::string,std::map<std::string, method *> > methods;
	std::map<std::string,field*> fields;
	std::map<std::string,field*> static_fields;
	const_pool * cpool = nullptr;

	char * static_members = nullptr;
	size_t static_member_size = 0;
	size_t member_size = 0;
	jreference mirror = 0;

	bool is_class() { return true; }
	size_t static_size() { return static_member_size; }
	size_t size() const { return member_size;}
	method * get_init_method();
	method * get_clinit_method();
	method * lookup_method(const std::string & name, const std::string & discriptor);
	field * lookup_field(const std::string & name);
};

struct array_claxx : public claxx
{
	bool is_array() { return true; }
	claxx * component = nullptr;
	jtype componen_type;
};

