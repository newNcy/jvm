
#include "thread.h"
#include "attribute.h"
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
#include <cstdlib>
#include <ffi-x86_64.h>
#include <ffi.h>
#include <stdexcept>
#include <utility>
#include <vector>

frame::frame(frame * caller, method * to_call, thread * context ):caller_frame(caller), current_method(to_call), current_class(to_call->owner)
{
	if (!current_class) { 
		return;
	}
	current_thread = context;
	current_const_pool = current_class->cpool;
	auto code_it = current_method->attributes.find("Code");
	if (code_it == current_method->attributes.end()) {
		stack = new operand_stack(2);
		locals = new any_array(2);
		return;
	}
	code = (code_attr*)code_it->second;
	
	pc.set_buf((const char*)code->code, code->code_length);
	stack = new operand_stack(code->max_stacks);
	locals = new any_array(code->max_locals);

	auto locals_table_it = current_method->attributes.find("LocalVariableTable");
	if (locals_table_it != current_method->attributes.end()) {
		locals_table = static_cast<local_variable_table_attr*>(locals_table_it->second);
	}
	gettimeofday(&start_time, nullptr);

}

void frame::throw_exception(const char * name)
{
	exception_occured = true;
	runtime_error = name;
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
	if (!current_frame) return;
	timeval end_time;
	gettimeofday(&end_time, nullptr);
	timeval & start_time = current_frame->start_time;
	frame * next = current_frame->caller_frame;
	//delete current_frame;
	current_frame = next;
}

bool thread::handle_exception()
{
	std::vector<std::pair<method*, u2>> unhandle_methods;
	jreference e = current_frame->stack->pop<jreference>();
	object * obj = memery::ref2oop(e);
	for (;current_frame;) {
		for (exception & e : current_frame->code->exceptions) {
			u2 cur_pc = current_frame->pc.pos();
			if (e.start_pc <= cur_pc && cur_pc <= e.end_pc) {
				current_frame->pc.pos( e.handler_pc);
				current_frame->stack->push(e);
				printf("%s handle by %s.%s:%s\n", obj->meta->name->c_str(), 
						current_frame->current_class->name->c_str(), 
						current_frame->current_method->name->c_str(), 
						current_frame->current_method->discriptor->c_str());
				return true;
			}
		}
		printf("%s unhandle by %s.%s:%s\n", obj->meta->name->c_str(), 
				current_frame->current_class->name->c_str(), 
				current_frame->current_method->name->c_str(), 
				current_frame->current_method->discriptor->c_str());
		unhandle_methods.push_back(std::make_pair(current_frame->current_method,current_frame->current_pc));
		current_frame = current_frame->caller_frame;
		pop_frame();
	}
	printf("unhandle exception %s at:\n", obj->meta->name->c_str());
	for (auto && m : unhandle_methods) {
		auto ln = m.first->attributes.find("LineNumberTable");
		printf("in %s.%s ",m.first->owner->name->c_str(), m.first->name->c_str());
		if (ln != m.first->attributes.end()) {
			auto table = static_cast<line_number_table_attr*>(ln->second)->line_number_table;
			auto line = table.find(m.second);
			if (line != table.end()) {
				printf("%d ", line->second);
			}
		}
		printf("\n");
	}
	return false;
}

void frame::print_stack() 
{
	printf("stack\t[%d/%d] ",stack->top_ptr, stack->size());
	if (stack->begin() != stack->end()) printf("|");
	for (any_array::iterator i = stack->begin(); i < stack->end(); i++) {
		if (stack->types[i-stack->begin()] == T_FLOAT) {
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
	if (!locals->size()) {
		printf("\n");
		return;
	}
	if (locals->begin() != locals->end()) printf("|");
	int idx = 0;
	for (any_array::iterator i = locals->begin(); i < locals->end(); i++) {
		if (locals_table) {
			printf("a%s:", current_const_pool->get(locals_table->local_variable_table[idx].name_index)->utf8_str->c_str());
		}
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

	uint32_t arg_count = m->arg_types.size() + 1 + m->is_static(); //Env指针,this/class指针
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
	if (m->is_static()) {
		arg_types[1] = &ffi_type_uint32;
		arg_ptrs[1] = alloca(arg_types[1]->size);
		*(jreference*)arg_ptrs[1] = m->owner->mirror;
		idx ++;
	}
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
			return ret = *(jvalue*)ret_ptr;
		}
	}
	return 0;
}



jvalue thread::call(method * m, operand_stack * args, bool is_interface, bool call_in_native)
{
	if (!m) return 0; 
	if (!args && current_frame) {
		args = current_frame->stack;
	}

	fprintf(stderr, "%s %s.%s \n", m->owner->name->c_str(), m->name->c_str(), m->discriptor->c_str());

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
			printf(" %s:", type_text[t]);
			if (t == T_FLOAT) {
				printf("%f ", temp_stack->get<jfloat>(idx));
			}else {
				printf("%d ", temp_stack->get<jint>(idx));
			}
			idx--;
		}
		printf(")\n");
	}
	fflush(stdout);

	jvalue ret = 0;

	if (is_interface || m->is_abstract()) {
		jreference objref = temp_stack->top<jreference>();
		object * oop = memery::ref2oop(objref);
		m = oop->meta->lookup_method(m->name->c_str(), m->discriptor->c_str());
	}

	if (m->is_native()) {
		ret = call_native(m, temp_stack);
	}else {
		//interface_method 
		frame * new_frame = new frame(current_frame, m, this);
		if (!new_frame->locals) abort();
		current_frame = new_frame;	
		depth ++;
		if (temp_stack) {
			int idx = 0;
			for (auto  t : m->arg_types) {
				if (t == T_LONG || t == T_DOUBLE) { 
					new_frame->locals->put(temp_stack->pop<jlong>(),idx);
					idx += 2;
				}else {
					new_frame->locals->put(temp_stack->pop<jint>(),idx);
					idx ++;
				}
			}
		}
		try {
			if (!m->is_static() && new_frame->locals->get<jreference>(0) == null) {
				throw_exception_to_java("java/lang/NullPointerException"); 
			}
			current_frame->exec(m->owner->name->c_str(), m->name->c_str());
		}catch(const char *  e) {
			throw_exception_to_java(e);
		}
		depth --;

		if (current_frame->exception_occured) {
			handle_exception();
		}else if (m->ret_type != T_VOID) {
			try {
				if (m->ret_type == T_LONG) {
					ret = current_frame->stack->pop<jlong>();
				}else if (m->ret_type == T_DOUBLE) {
					ret = current_frame->stack->pop<jdouble>();
				}else if (m->ret_type == T_FLOAT) {
					ret = current_frame->stack->pop<jfloat>();
				}else {
					ret = current_frame->stack->pop<jint>();
				}
			}catch (const std::string & e)  {
				throw_exception_to_java(e);
			}
		}
		pop_frame();
	}
	printf("%s.%s%s exit ", m->owner->name->c_str(), m->name->c_str(), m->discriptor->c_str());
	if (!call_in_native && m->ret_type != T_VOID) {
		if (m->ret_type == T_LONG) {
			current_frame->print_stack();
			printf("return long %ld\n", ret.j);
			current_frame->stack->push(ret.j);
		}else if (m->ret_type == T_DOUBLE) {
			current_frame->stack->push(ret.d);
			printf("return %lf\n", ret.d);
		}else if (m->ret_type == T_FLOAT) {
			current_frame->stack->push(ret.f);
			printf("\e[34mreturn %f\n\e[0m", ret.f);
		}else {
			current_frame->stack->push(ret.i);
			printf("\e[34mreturn %d\n\e[0m", ret.i);
		}
	}else {
		printf("\n");
	}
	if (temp_stack) delete temp_stack;
	return ret;
}

jreference thread::create_string(const std::string & bytes)
{
	return get_env()->create_string(bytes);
}
void thread::throw_exception_to_java(const std::string & name)
{
	claxx * c = get_env()->get_vm()->get_class_loader()->load_class(name, this);
	current_frame->stack->push(memery::alloc_heap_object(c));
	handle_exception();
}

thread::thread(jvm * this_vm): runtime_env(this_vm, this)
{
	auto java_lang_Thread = this_vm->get_class_loader()->load_class("java/lang/Thread", this);
	this->mirror = java_lang_Thread->instantiate(this);
	auto f = java_lang_Thread->lookup_field("priority");
	get_env()->set_object_field(this->mirror, f, 5);
}
