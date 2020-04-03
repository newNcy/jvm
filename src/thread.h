#pragma once

#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <sys/time.h>
#include <stack>
#include "class.h"
#include "memery.h"
#include "native.h"


struct thread;
typedef jint local_entry;

struct any_array
{
	uint32_t N = 0;
	jint * data = nullptr;
	uint32_t size() { return N; }
	any_array(uint32_t size):N(size) 
	{ 
		if (!N) return;
		data = (jint*)memery::alloc_meta_space(sizeof(jint)*N);
		memset(data,0, N*sizeof(jint));
	}
	~any_array()
	{
		if (data) {
			memery::dealloc_meta_space(data);
			data = nullptr;
		}
	}

	template <typename T>
	void put(T t, int idx)
	{
		//std::cout<<idx<<" put "<<t<<std::endl;
		int size = 1 + (sizeof(T) > sizeof(jint));
		if (0 <= idx && idx <= N - size) {
			*((T*)(data+idx)) = t;
		}
	}
	
	template <typename T>
	T get(int idx)
	{
		//td::cout<<idx<<" get"<<std::endl;
		int size = 1 + (sizeof(T) > sizeof(jint));
		if (0 <= idx && idx <= N - size) {
			return *(T*)(data+idx);
		}
	}
	jint * begin() { return data; }
	virtual jint * end() { return data+N; }
	using iterator = jint *;
};


struct operand_stack : public any_array
{
	operand_stack(uint32_t max): any_array(max) {}
	bool empty() const { return top_ptr == 0; }
	jint top_ptr = 0;
	std::vector<jtype> types;
	
	template <typename T>
	void push(T t)
	{
		int s = 1 + (sizeof(T) > sizeof(jint));
		if (top_ptr + s > size()) {
			throw std::runtime_error("stack over flow");
		}
		put<T>(t, top_ptr);
		types.push_back((jtype)jtype_value_traits<T>::type_value);
		top_ptr += s;
	}
	template <typename T>
	T pop()
	{
		T t = top<T>();
		int size = 1 + (sizeof(T) > sizeof(jint));
		top_ptr -= size;
		if (top_ptr < 0) {
			throw std::runtime_error("stack empty");
		}
		types.pop_back();
		return t;
	}
	template <typename T>
	T top()
	{
		int size = 1 + (sizeof(T) > sizeof(jint));
		return get<T>(top_ptr - size);
	}
	iterator end() { 
		iterator it = begin() + top_ptr; 
		return it;
	}
};


struct frame 
{
	const_pool		* current_const_pool = nullptr;
	method			* current_method = nullptr;
	thread			* current_thread = nullptr;
	claxx			* current_class = nullptr;
	frame			* caller_frame = nullptr;
	any_array		* locals = nullptr;
	operand_stack	* stack = nullptr;
	code_attr		* code = nullptr;

	void setup_args();
	void pc_offset(short offset) 
	{
		pc.rd += offset - 1;
	}
	void print_stack();
	void print_locals();
	byte_stream pc;
	timeval start_time;
	public:
	frame(frame * caller, method * to_call, thread *);
	void exec(const char *, const char *);//for gdb
	~frame();
};

struct jvm;
struct thread
{
	frame			* current_frame = nullptr;
	environment runtime_env;
	thread(jvm * this_vm): runtime_env(this_vm, this) {}
	environment * get_env() { return &runtime_env; }
	jreference create_string(const std::string & bytes)
	{
		return 0;
	}
	jvalue call_native(method * m,operand_stack * args = nullptr);
	jvalue call(method * m,operand_stack * args = nullptr);

	void pop_frame();
	void run();
	bool handle_exception();
	void start() 
	{
		run();
	}
};



