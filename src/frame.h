#pragma once
#include "class.h"
#include "memery.h"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sys/time.h>
#include <sys/wait.h>
#include <unordered_map>
#include "util.h"

/*
 * @brif 局部变量表
 */
class array
{
	public:
		using slot_type = jint;
		using iterator = slot_type*;
		uint32_t size() const { return end_address - start_address; }
		iterator begin()	{ return start_address; }
		iterator end()		{ return end_address < start_address?start_address:end_address; }

		array(uint32_t n);
		array(iterator s, iterator e);
		array(iterator s, uint32_t n);
		void copy(iterator s, iterator e, iterator w = nullptr);
		void copy(iterator s, uint32_t n);
		virtual ~array();

		template <typename T> T get(jint index);
		template <typename T> void put(T t, jint index);
		
		template <typename T> struct slot_need { constexpr static uint32_t value = 1+(sizeof(T)-1)/sizeof(slot_type); };
		static uint32_t slot_need2 (jtype t) { return t?1+(type_size[t]-1)/sizeof(slot_type):0;};
		
		void print();
	protected:
		template <typename T>
		void check_iter(iterator it, const char * msg = "invalid index ");
		iterator start_address	= nullptr;
		iterator end_address	= nullptr;
};


template <typename T> void array::check_iter(iterator it, const char * msg)
{
	if (it < begin() || end() < it + slot_need<T>::value) {
		throw std::runtime_error(std::string(msg) + std::to_string(it - begin()) + "/" + std::to_string(size()));
	}
}

template <typename T> T array::get(jint index)
{
	iterator pos = start_address + index;
	check_iter<T>(pos);
	element_type <T> * ptr = (element_type<T>*)pos;
	return *ptr;
}

template <typename T> void array::put( T t, jint index)
{
	iterator pos = start_address + index;
	check_iter<T>(pos);
	element_type<T> * ptr = (element_type<T>*)pos;
	*ptr = t;
}

/*
 * @brif 实现操作栈
 */
class array_stack : public array
{
	public:
		array_stack(uint32_t n);

		iterator top_pos() const { return top_adderss; }
		uint32_t depth() { return top_pos() - begin(); }
		template <typename One> One top();

		template <typename One> One pop();
		template <typename One> void pop(One & one);
		template <typename ... More, typename One> void pop( More & ... more, One & one);

		template <typename One> void push(One t);
		template <typename One, typename ... More> void push(One t, More ... more);


		void print();
	private:
		iterator top_adderss = nullptr; //下一个能存放的位置
};


template <typename One> One array_stack::top()
{
	One one = array::get<One>(depth() - slot_need<One>::value);
	return one;
}

template <typename One> One array_stack::pop()
{
	iterator after_top = top_adderss - slot_need<One>::value;
	check_iter<One>(after_top);
	One one = top<One>();
	top_adderss = after_top;
	return one;
}

template <typename One> void array_stack::pop(One & one)
{
	one = pop<One>();
}

template <typename ... More, typename One> void array_stack::pop( More & ... more, One & one)
{
	pop(one);
	pop(more ...);
}

template <typename One> void array_stack::push(One t)
{
	array::put(t, depth());
	top_adderss = begin() + depth() + slot_need<One>::value;
}

template <typename One, typename ... More> void array_stack::push(One one, More ... more)
{
	push(one);
	push(more ...);
}

struct frame 
{
	local_variable_table_attr * locals_table = nullptr;
	const_pool		* current_const_pool = nullptr;
	method			* current_method = nullptr;
	thread			* current_thread = nullptr;
	claxx			* current_class = nullptr;
	frame			* caller_frame = nullptr;
	array_stack		* locals = nullptr; //本地变量也用栈，反正可以当成表用
	array_stack		* stack = nullptr;
	code_attr		* code = nullptr;
	u4 current_pc   = 0;
	bool exception_occured = false;
	std::string runtime_error;
	void throw_exception(const char * name);
	void setup_args();
	void print_stack();
	void print_locals();
	byte_stream pc;
	timeval start_time;
	public:
	frame(thread *, method * to_call, array_stack * args); //出栈顺序就是参数顺序
	jvalue interpreter(const char * clazz, const char * name, const char * discriptor);//for gdb
	jvalue native();//for gdb
	~frame();
};


