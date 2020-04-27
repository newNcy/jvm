#include "frame.h"
#include "class.h"
#include "log.h"
#include "thread.h"
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <stdio.h>
#include <termios.h>
#include "jvm.h"

array::array(uint32_t n)
{
	if (n <= 0) return;
	start_address = new jint[n]();
	end_address = start_address + n;
}

array::array(array::iterator s, iterator e)
{
	if (!s || !e) return;
	if (e <= s) return;

	array(e-s);
	copy(s, e);
}

void array::copy(iterator s, iterator e, iterator w)
{
	if (!s || !e) return;
	if (e - s > size()) return;
	if (!w) w = begin();
	for (iterator r = s; r != e; ++ r) {
		*w ++ = *r;	
	}
}

void array::copy(iterator s, uint32_t n)
{
	copy(s, s+n);
}

array::array(iterator s, uint32_t n) 
{
	if (!s) return;
	array(s, s + n);
}

void array::print()
{
	for (auto i : *this) {
		printf("%d ", i);
	}
}
array::~array()
{
	if (start_address) {
		delete [] start_address;
	}
	start_address = nullptr;
	end_address = nullptr;
}

array_stack::array_stack(uint32_t n):array(n)
{
	top_adderss = begin();
}

void array_stack::print()
{
	iterator i = top_pos() - 1;
	while (i != begin()-1) {
		printf("%d ", *i);
		-- i;
	}
}

frame::frame(thread * context,  method * to_call, array_stack * args)
{
	if (!context || !to_call || !to_call->owner) {
		throw std::runtime_error("call incomplete");
	}

	current_thread		= context;
	current_method		= to_call;
	current_class		= to_call->owner;
	current_const_pool	= current_class->cpool;
	caller_frame		= current_thread->current_frame;

	/*
	 * 到这里铁可以被执行.分java/native
	 */
	
	auto code_it = current_method->attributes.find("Code");
	if (code_it != current_method->attributes.end()) {
		code = (code_attr*)code_it->second;
		pc.set_buf((const char*)code->code, code->code_length);
		stack = new array_stack(code->max_stacks);
		locals = new array_stack(code->max_locals);
	}else if (!current_method->is_native()) {
		throw std::runtime_error("No Code");
	}

	/*
	 * 把实参放到本地变量表中
	 */
	if (args) {
		//args->array::print();
		array::iterator s = args->top_pos() - current_method->arg_space;
		//log::trace("args top %d end %d", args->top_pos() - args->begin(), args->end() - args->begin());
		array::iterator w = nullptr; 
		if (current_method->is_native()) {
			locals = new array_stack(current_method->arg_space+1);
			stack  = new array_stack(2); //native return
		}
		if (!current_method->is_static()) s --;

		if (locals) {
			locals->copy(s, args->top_pos(), w);
			while (args->top_pos() != s) {
				args->pop<jint>();
			}
		}
	}

	gettimeofday(&start_time, nullptr);
}

jvalue frame::native()
{
	vm_native::native_metod fn = current_method->native_method;
	if (!fn) {
		fn = current_thread->get_env()->get_vm()->vm_native_methods.find_native_implement(current_method);
		current_method->native_method = fn;
	}
	if (!fn) {
		//后面改成抛异常
		throw "java/lang/UnsatisfiedLinkError";
	}

	uint32_t arg_count = current_method->arg_types.size() + 2;//Env指针,this/class指针
	ffi_type ** arg_types = (ffi_type**)alloca(sizeof(ffi_type)*arg_count);
	void ** arg_ptrs = (void**)alloca(sizeof(void*)*arg_count);
	void * ts [] = {
		&ffi_type_uint8, 
		&ffi_type_uint16, 
		&ffi_type_float,
		&ffi_type_double,
		&ffi_type_uint8, 
		&ffi_type_uint16,
		&ffi_type_sint32,
		&ffi_type_sint64,
		&ffi_type_uint32,
	};

	arg_types[0] = &ffi_type_pointer;
	arg_ptrs[0] = alloca(arg_types[0]->size);
	*(environment**)arg_ptrs[0] = current_thread->get_env();

	int arg_idx = 0;
	jreference class_or_this = current_method->is_static()?current_class->mirror : locals->get<jreference>(arg_idx++);
	arg_types[1] = &ffi_type_uint32;
	arg_ptrs[1] = alloca(arg_types[1]->size);
	*(jreference*)arg_ptrs[1] = class_or_this;
	int idx = 2;
	for (auto & t : current_method->arg_types) {
		arg_types[idx] = (ffi_type*)ts[t-T_BOOLEAN];
		arg_ptrs[idx] = alloca(arg_types[idx]->size);

		jvalue v = 0;//用于在native类型与java类型在栈中的布局转换 如1byte的boolean在栈中是4字节
		if (type_size[t] > sizeof(jint)) {
			v = locals->get<jlong>(arg_idx);
			arg_idx += 2;
		}else {
			v = locals->get<jint>(arg_idx);
			arg_idx ++;
		}
		if (t == T_BOOLEAN)		*(jboolean*)arg_ptrs[idx]	= v.z;
		else if (t == T_CHAR)	*(jchar*)arg_ptrs[idx]		= v.c;
		else if (t == T_FLOAT)	*(jfloat*)arg_ptrs[idx]		= v.f;
		else if (t == T_DOUBLE) *(jdouble*)arg_ptrs[idx]	= v.d;
		else if (t == T_BYTE)	*(jbyte*)arg_ptrs[idx]		= v.b;
		else if (t == T_SHORT)	*(jshort*)arg_ptrs[idx]		= v.s;
		else if (t == T_INT)	*(jint*)arg_ptrs[idx]		= v.i;
		else if (t == T_LONG)	*(jlong*)arg_ptrs[idx]		= v.j;
		else if (t == T_OBJECT)	*(jreference*)arg_ptrs[idx] = v.l;
		else {
		}
		idx ++;
	}

	ffi_type * ret_type = nullptr;
	if (current_method->ret_type == T_VOID) {
		ret_type = &ffi_type_void;
	}else {
		//log::debug("return type %s", type_text[current_method->ret_type]);
		fflush(stdout);
		ret_type = (ffi_type*)ts[current_method->ret_type - T_BOOLEAN];
	}

	ffi_cif cif;
	ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, arg_count, ret_type, arg_types);
	jvalue ret = 0;
	if (status == FFI_OK) {
		ffi_call(&cif, fn, &ret, arg_ptrs);
	}
	return ret;
}

void frame::throw_exception(const char * name)
{
	exception_occured = true;
	runtime_error = name;
}

frame::~frame()
{
	if (locals) delete locals;
	if (stack) delete stack;

}

void frame::print_stack() 
{
	printf("[%d/%d]\t",stack->depth(), stack->size());
	stack->print();
	printf("\n");
}
void frame::print_locals()
{
	printf("[%d]\t", locals->size());
	if (!locals->size()) {
		printf("\n");
		return;
	}
	locals->array::print();

	printf("\n");
}

