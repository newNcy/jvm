#pragma once

#include <stdexcept>
#include <sys/types.h>
#include <stdint.h>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include "singleton.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

enum tag_enum 
{
	CONSTANT_Utf8 = 1,
	CONSTANT_Integer = 3,
	CONSTANT_Float,
	CONSTANT_Long,
	CONSTANT_Double,
	CONSTANT_Class,
	CONSTANT_String,
	CONSTANT_FieldRef,
	CONSTANT_MethodRef,
	CONSTANT_InterfaceMethodRef,
	CONSTANT_NameAndType,
	CONSTANT_MethodHandle = 15,
	CONSTANT_MethodType,
	CONSTANT_InvokeDynamic = 18,
};

struct cp_info 
{
	u1 tag;
};

template <tag_enum t>
struct cp_info_base : public cp_info
{
	enum { TAG = t };
};

struct cp_class :public cp_info_base<CONSTANT_Class>
{
	u2 name_index;
};

template <tag_enum t>
struct name_and_type_ref : public cp_info_base<t>
{
	u2 class_index;
	u2 name_and_type_index;
};

typedef name_and_type_ref<CONSTANT_FieldRef> cp_field_ref;
typedef name_and_type_ref<CONSTANT_MethodRef> cp_method_ref;
typedef name_and_type_ref<CONSTANT_InterfaceMethodRef> cp_interface_method_ref;

struct cp_string : public cp_info_base<CONSTANT_String>
{
	u2 string_index;
};

template <tag_enum t>
struct u4_num : public cp_info_base<t>
{
	u4 bytes;
};
typedef u4_num<CONSTANT_Integer> cp_int;
typedef u4_num<CONSTANT_Float> cp_float;

template <tag_enum t>
struct u8_num : public cp_info_base<t>
{
	u8 bytes;
};
typedef u8_num<CONSTANT_Long> cp_long;
typedef u8_num<CONSTANT_Double> cp_double;


struct cp_name_and_type : public cp_info_base<CONSTANT_NameAndType>
{
	u2 name_index;
	u2 discriptor_index;
};

struct cp_method_handle : public cp_info_base<CONSTANT_MethodHandle>
{
	u1 reference_kind;
	u2 reference_index;
};

struct cp_method_type : public cp_info_base<CONSTANT_MethodType>
{
	u2 discriptor_index;
};

struct cp_invoke_dynamic : public cp_info_base<CONSTANT_InvokeDynamic>
{
	u2 boostrap_method_attr_index;
	u2 name_and_type_index;
};

struct cp_utf8 : public cp_info_base<CONSTANT_Utf8>
{
	u2 length;
	u1 bytes[];
	bool equals(const char * str) const
	{
		if (!str) return false;
		return !strcmp(c_str(), str);
	}
	bool equals(const cp_utf8 * b) const 
	{
		return b && (b == this || equals(b->c_str()));
	}
	char at(int idx)
	{
		return (char)bytes[idx];
	}
	const char * c_str() const { return (const char *)bytes; };
	static u4 hash_for_max(const char * s, u4 max)
	{
		u4 ret = 0;
		int i = 0;
		while (s[i]) {
			ret += i*s[i] + i;
			ret %= max;
			i ++;
		}
		return  ret;
	}
};

typedef cp_utf8 symbol;

struct symbol_pool : public singleton<symbol_pool>
{
	public:
		symbol * put(symbol * sym);
		symbol * put(const std::string sym);
	private:
		std::map<std::string, symbol*> symbols;
};


class byte_stream
{
	private:
		char * buf = nullptr;
		bool is_ref = false;
		int max = 0;
		int position = 0;
	public:
		void set_buf(const char * buff, int length)
		{
			buf = const_cast<char*>(buff);
			position = 0;
			max = length-1;
		}
		int pos(int p = -1) { return p == -1 ? position : position = p; }
		void pos_offset(int off) { position += off; }
		int value() const { return max >= position ? max - position + 1: 0; }
		template <typename T>
			T get();

		int read(u1 * out, size_t size)
		{
			if (!out) {
				position += size;
				return 0;
			}
			if (size > 0 && value() >= size) {
				memcpy(out, buf + position, size);
				position += size;
				return size;
			}else {
				return 0;
			}
		}

		~byte_stream()
		{
		}
};

	template <typename T>
T byte_stream::get()
{
	if (value() < sizeof(T)) throw std::runtime_error(std::string("max:")+std::to_string(max)+" pos:"+std::to_string(position) + " get:" + std::to_string(sizeof(T)));
	T t = 0;
	for (int i = sizeof(T) ; i > 0; i --) {
		t |= (buf[position++]&0xff)<<((i-1)*8);
	}
	return t;
}

