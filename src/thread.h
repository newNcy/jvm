#pragma once

#include <vector>
#include <iostream>
#include <sys/time.h>
#include <stack>
#include "class.h"
#include "memery.h"


struct thread;
typedef jint local_entry;

struct any_array
{
	uint32_t N = 0;
	jint * data = nullptr;
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
	jint * end() { return data+N-1; }
};

struct operand_stack : public any_array
{
	operand_stack(uint32_t max): any_array(max) {}


	bool empty() const { return top_ptr == 0; }
	int top_ptr = 0;

	template <typename T>
	void push(T t)
	{
		int size = 1 + (sizeof(T) > sizeof(jint));
		put<T>(t, top_ptr);
		top_ptr += size;
	}
	template <typename T>
	T pop()
	{
		T t = top<T>();
		int size = 1 + (sizeof(T) > sizeof(jint));
		top_ptr -= size;
		return t;
	}
	template <typename T>
	T top()
	{
		int size = 1 + (sizeof(T) > sizeof(jint));
		return get<T>(top_ptr - size);
	}
};


struct frame 
{
	const_pool		* current_const_pool = nullptr;
	method			* current_method = nullptr;
	claxx			* current_class = nullptr;
	frame			* caller_frame = nullptr;
	any_array		* locals = nullptr;
	operand_stack	* stack = nullptr;
	code_attr		* code = nullptr;

	byte_stream pc;
	timeval start_time;
	public:
	frame(frame * caller, method * to_call);
	void exec();
	~frame();
};

struct jvm;
struct thread
{
	jvm * vm = nullptr;
	frame * current_frame = nullptr;
	claxx * current_class = nullptr;
	method * current_method = nullptr;
	const_pool * current_const_pool = nullptr;
	byte_stream * code = nullptr;
	void pc_offset(u2 offset) 
	{
		if (code) code->rd += offset - 1;
	}
	jreference create_string(const std::string & bytes)
	{
		return 0;
	}
	void push_frame(method * m);

	void pop_frame();
	
	void run();
	bool handle_exception();
	void start(method * entry) 
	{
		if (!entry) return;
		push_frame(entry);
		run();
	}
};



