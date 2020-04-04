
#include "thread.h"
#include "class.h"
#include "classfile.h"
#include "interpreter.h"
#include "jvm.h"
#include "memery.h"
#include "native.h"
#include "object.h"
#include "classloader.h"

#include <alloca.h>
#include <cstdint>
#include <cstdio>
#include <ffi-x86_64.h>
#include <ffi.h>
#include <stdexcept>

frame::frame(frame * caller, method * to_call, thread * context ):caller_frame(caller), current_method(to_call), current_class(to_call->owner)
{
	if (!current_class) return;
	current_thread = context;
	current_const_pool = current_class->cpool;

	auto code_it = current_method->attributes.find("Code");
	if (code_it == current_method->attributes.end()) {
		return;
	}
	code = (code_attr*)code_it->second;
	
	pc.set_buf((const char*)code->code, code->code_length);
	stack = new operand_stack(code->max_stacks);
	locals = new any_array(code->max_locals);

	gettimeofday(&start_time, nullptr);

}

void frame::setup_args()
{
	int idx = 0;
	int argid = 1;
	printf("add frame [%s.%s] arg num %ld\n", current_class->name->c_str(), current_method->name->c_str(), current_method->arg_types.size());
	for (auto type : current_method->arg_types) {
		if (idx == current_method->arg_types.size()) break;
		switch (type) {
			case T_LONG:
				{
					jlong v = stack->pop<jlong>();
					printf("arg%d long:%ld\n", argid++, v);
					locals->put<jlong>(v, idx);
					idx += 2;
				}
				break;
			case T_DOUBLE:
				{
					jdouble v = stack->pop<jdouble>();
					printf("arg%d double:%lf\n", argid++, v);
					locals->put<jdouble>(v, idx);
					idx += 2;
				}
				break;
			case T_FLOAT:
				{
					jfloat v = stack->pop<jfloat>();
					printf("arg%d float:%f\n", argid++, v);
					locals->put(v, idx);
					idx ++;
				}
				break;
			default:
				{
					jint v = stack->pop<jint>();
					printf("arg%d int:%d\n", argid++, v);
					locals->put<jint>(v, idx);
					idx ++;
				}
		}
	}
	printf("-------------------\n");

}

frame::~frame()
{
}


void thread::run()
{
	while (current_frame) {
	}
}

void thread::pop_frame()
{
	timeval end_time;
	gettimeofday(&end_time, nullptr);
	printf("call end\n");
	timeval & start_time = current_frame->start_time;
	frame * next = current_frame->caller_frame;
	delete current_frame;
	current_frame = next;
}

bool thread::handle_exception()
{
	std::stack<method*> unhandle_methods;
	jreference e = current_frame->stack->top<jreference>();
	object * obj = memery::ref2oop(e);
	for (;current_frame;) {
		for (exception & e : current_frame->code->exceptions) {
			u2 cur_pc = current_frame->pc.pos();
			if (e.start_pc <= cur_pc && cur_pc <= e.end_pc) {
				current_frame->pc.pos( e.handler_pc);
				printf("%s handle by %s.%s:%s\n", obj->meta->name->c_str(), 
						current_frame->current_class->name->c_str(), 
						current_frame->current_method->name->c_str(), 
						current_frame->current_method->discriptor->c_str());
				return true;
			}
		}
		unhandle_methods.push(current_frame->current_method);
		pop_frame();
	}
	printf("unhandle exception %s at:\n", obj->meta->name->c_str());
	while (!unhandle_methods.empty()) {
		auto m = unhandle_methods.top();
		unhandle_methods.pop();
		printf("in %s\n", m->name->c_str());
	}
	return false;
}

void frame::print_stack() 
{
	printf("stack\t[%d] ",stack->size());
	if (stack->begin() != stack->end()) printf("|");
	for (any_array::iterator i = stack->begin(); i != stack->end(); i++) {
		if (stack->types[i-stack->begin()] == T_LONG) {
			printf("%ld|", *(jlong*)i);
			i++;
		}else if (stack->types[i-stack->begin()] == T_FLOAT) {
			printf("%f|", *(jfloat*)i);
		}else if (stack->types[i-stack->begin()] == T_DOUBLE) {
			printf("%lf|", *(jdouble*)i);
			i++;
		}else {
			printf("%d|", *(jint*)i);
		}
	}
	printf("\n");
}
void frame::print_locals()
{
	printf("locals\t[%d] ", locals->size());
	if (locals->begin() != locals->end()) printf("|");
	for (any_array::iterator i = locals->begin(); i != locals->end(); i++) {
		printf("%d|", *i);
	}
	printf("\n");
}

jvalue thread::call_native(method * m,operand_stack * args)
{
	printf("try to call native method %s\n",vm_native::trans_method_name(m).c_str());
	vm_native::native_metod fn = m->native_method;
	if (!fn) {
		fn = get_env()->get_vm()->vm_native_methods.find_native_implement(m);
		m->native_method = fn;
	}
	if (!fn) {
		//后面改成抛异常
		printf("native method %s not found\n",vm_native::trans_method_name(m).c_str());
		throw std::runtime_error("native method");
		return 0;
	}
	uint32_t arg_count = m->arg_types.size() + 1; //Env指针 暂时丢个thread*给它
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
	*(environment**)arg_ptrs[0] = get_env();

	int idx = 1;
	for (auto & t : m->arg_types) {
		arg_types[idx] = (ffi_type*)ts[t-T_BOOLEAN];
		arg_ptrs[idx] = alloca(arg_types[idx]->size);
		if (t == T_BOOLEAN)		*(jboolean*)arg_ptrs[idx] = args->pop<jboolean>();
		else if (t == T_CHAR)	*(jchar*)arg_ptrs[idx] = args->pop<jchar>();
		else if (t == T_FLOAT)	*(jfloat*)arg_ptrs[idx] = args->pop<jfloat>();
		else if (t == T_DOUBLE) *(jdouble*)arg_ptrs[idx] = args->pop<jdouble>();
		else if (t == T_BYTE)	*(jbyte*)arg_ptrs[idx] = args->pop<jbyte>();
		else if (t == T_SHORT)	*(jshort*)arg_ptrs[idx] = args->pop<jshort>();
		else if (t == T_INT)	*(jint*)arg_ptrs[idx] = args->pop<jint>();
		else if (t == T_LONG)	*(jlong*)arg_ptrs[idx] = args->pop<jlong>();
		else if (t == T_OBJECT)	*(jreference*)arg_ptrs[idx] = args->pop<jreference>();
		idx ++;
	}
	ffi_type * ret_type = nullptr;
	if (m->ret_type == T_VOID) {
		ret_type = &ffi_type_void;
	}else {
		ret_type = (ffi_type*)ts[m->ret_type - T_BOOLEAN];
	}

	ffi_cif cif;
	ffi_status status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, arg_count, ret_type, arg_types);
	jvalue ret;
	if (status == FFI_OK) {
		void * ret_ptr = nullptr;
		if (ret_type->size) {
			ret_ptr = alloca(ret_type->size);
		}
		ffi_call(&cif, fn, ret_ptr, arg_ptrs);
		if (ret_ptr) {
			return *(jvalue*)ret_ptr;
		}
	}
	return 0;
}



jvalue thread::call(method * m, operand_stack * args, bool is_interface)
{
	if (!m) return 0; 
	if (!args && current_frame) {
		args = current_frame->stack;
	}


	operand_stack * temp_stack = nullptr;
	if (args) {
		temp_stack = new operand_stack(args->size());
		for (int idx = m->arg_types.size()-1; idx >= 0; idx --) {
			if (m->arg_types[idx] == T_LONG || m->arg_types[idx] == T_DOUBLE)  temp_stack->push(args->pop<jlong>());
			else temp_stack->push(args->pop<jint>());
		}
		printf("with args(");
		int idx = m->arg_types.size() - 1;
		for (auto t : m->arg_types) {
			printf(" %s:", type_text[t-T_BOOLEAN]);
			printf("%d ", temp_stack->get<jint>(idx));
			idx--;
		}
		printf(")\n");
	}
	fflush(stdout);

	jvalue ret = 0;
	if (m->is_native()) {
		ret = call_native(m, temp_stack);
	}else {
		//interface_method 
		if (is_interface) {
			jreference objref = current_frame->locals->get<jreference>(0);
			object * oop = memery::ref2oop(objref);
			m = oop->meta->lookup_method(m->name->c_str(), m->discriptor->c_str());
		}
		printf("call \e[34m%s.%s\e[0m \e[35m%s\e[0m\n", m->owner->name->c_str(), m->name->c_str(), m->discriptor->c_str());
		frame * new_frame = new frame(current_frame, m, this);
		if (temp_stack) {
			int idx = 0;
			for (auto && t : m->arg_types) {
				if (t == T_LONG || t == T_DOUBLE) { 
					new_frame->locals->put(temp_stack->pop<jlong>(),idx);
					idx += 2;
				}else {
					new_frame->locals->put(temp_stack->pop<jint>(),idx);
					idx ++;
				}
			}
		}
		current_frame = new_frame;
		current_frame->exec(m->owner->name->c_str(), m->name->c_str());
		pop_frame();
	}
	if (m->ret_type != T_VOID) {
		if (m->ret_type == T_LONG || m->ret_type  == T_DOUBLE) {
			current_frame->stack->push(ret.j);
			printf("\e[34mreturn %ld\n", current_frame->stack->top<jlong>());
		}else {
			current_frame->stack->push(ret.i);
			printf("\e[34mreturn %d\e[0m\n", current_frame->stack->top<jint>());
		}
	}
	if (temp_stack) delete temp_stack;
	return ret;
}
	
jreference thread::create_string(const std::string & bytes)
{
	return get_env()->create_string(bytes);
}
