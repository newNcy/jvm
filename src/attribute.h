#pragma once
#include "classfile.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <map>

struct attribute
{
	cp_utf8 * name = nullptr;
};

struct attribute_holder
{
	std::vector<attribute*> attributes;

	template <typename T>
	std::vector<T*> get_attributes();
};

template <typename T> 
struct attribute_name_helper
{
	static const char * name;
};


template <typename T>
const char * attribute_name()
{
	return attribute_name_helper<T>::name;
}


/*
 * 需要注意属性得数量限制，有些最多只有一个，有些是正好一个
 */
template <typename T>
inline std::vector<T*> attribute_holder::get_attributes()
{
	std::vector<T*> ret;
	for (auto attr : attributes) {
		if (attr->name->equals(attribute_name<T>())) {
			ret.push_back(static_cast<T*>(attr));
		}
	}
	return ret;
}


struct constant_attr : public attribute
{
	u2 constantvalue_index;
};


struct exception
{
	u2 start_pc;
	u2 end_pc;
	u2 handler_pc;
	u2 catch_type;
};

struct code_attr: public attribute, attribute_holder
{
	u2 max_stacks;
	u2 max_locals;
	u4 code_length;
	u1 * code = nullptr;
	std::vector<exception> exceptions;
};


struct source_attr : public attribute
{
	u2 sourcefile_index;
};
enum
{
	ITEM_Top,
	ITEM_Integer,
	ITEM_Float,
	ITEM_Double,
	ITEM_Long,
	ITEM_Null,
	ITEM_UninitializedThis,
	ITEM_Object,
	ITEM_Uninitialized,
};
struct verification_type_info 
{
	u1 tag;
	union {
		u2 cpool_index;
		u2 offset;
	};
};

enum 
{
	SAME,
	SAME_LOCALS_1_STACK_ITEM,
	SAME_LOCALS_1_STACK_ITEM_EXTENDED,
	CHOP,
	SAME_FRAME_EXTENDED,
	APPEND,
	FULL_FRAME,
};
struct stack_map_frame
{
	u1 frame_type;
	u2 offset_delta;
	std::vector<verification_type_info> stack;
	std::vector<verification_type_info> locals;
	stack_map_frame() {}
	~stack_map_frame() {}
};

struct stack_map_table_attr : public attribute
{
	std::vector<stack_map_frame> entries;
};

struct exceptions_attr : public attribute
{
	std::vector<u2> exception_index_table;
};


struct inner_class
{
	u2 inner_class_info_index;
	u2 outer_class_info_index;
	u2 inner_class_name_index;
	u2 inner_class_access_flags;
};
struct inner_classes_attr  : public attribute
{
	std::vector<inner_class> classes;
};

struct enclosing_method_attr : public attribute
{
	u2 class_index;
	u2 method_index;
};

struct signature_attr : public attribute
{
	u2 signature_index;
};


struct line_number_table_attr : public attribute
{
	std::map<u2,u2> line_number_table;
};


struct local_variable
{
	u2 start_pc;
	u2 length;
	u2 name_index;
	u2 discriptor_index;
	u2 index;
};

struct local_variable_table_attr : public attribute
{
	std::vector<local_variable> local_variable_table;
};

struct local_variable_type_table_attr : public attribute
{
	std::vector<local_variable> local_variable_type_table;
};

struct deprecated_attr : public attribute {};

struct annotation;
struct element_value 
{
	u1 tag;
	union 
	{
		u2 const_value_index;
		struct 
		{
			u2 type_name_index;
			u2 const_name_index;
		}enum_const_value;
		u2 class_info_index;
		annotation * annotation_ptr = nullptr;
		std::vector<element_value *> array_value; 
	};
	element_value() {}
	~element_value() {}
};

struct annotation 
{
	struct element_value_pair
	{
		u2 element_name_index;
		element_value * value = nullptr;
	};
	u2 type_index;
	std::vector<element_value_pair> element_value_pairs;
};
struct runtime_visible_anotations_attr : public attribute 
{
	std::vector<annotation*> anotation_ptrs;
};

struct annotation_default_attr : public attribute
{
	element_value * default_value = nullptr; 
};


struct bootstrap_method
{
	u2 bootstrap_method_ref;
	std::vector<u2> bootstrap_arguments;
};
struct bootstrap_methods_attr : public attribute
{
	std::vector<bootstrap_method> bootstrap_methods;
};


#define MAP_ATTR_NAME(ATTR, NAME) \
template <> struct attribute_name_helper<ATTR> { static constexpr const char * name = NAME;}

MAP_ATTR_NAME( enclosing_method_attr,	"EnclosingMethod"	);
MAP_ATTR_NAME( inner_classes_attr,		"InnerClasses"		);
MAP_ATTR_NAME( source_attr,				"SourceFile"		);
MAP_ATTR_NAME( code_attr,				"Code"				);
MAP_ATTR_NAME( line_number_table_attr,	"LineNumberTable"	);

#undef MAP_ATTR_NAME


