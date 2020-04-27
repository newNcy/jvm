
#include "thread.h"
#include "attribute.h"
#include "class.h"
#include "classfile.h"
#include "frame.h"
#include "interpreter.h"
#include "jvm.h"
#include "log.h"
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
#include <stdio.h>
#include <utility>
#include <vector>

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
	delete current_frame;
	current_frame = next;
}

bool thread::handle_exception()
{
	std::vector<std::pair<method*, u2>> unhandle_methods;
	jreference e = current_frame->stack->pop<jreference>();
	object * obj = memery::ref2oop(e);
	for (;current_frame;) {
		if (current_frame->code) {
			for (exception eh : current_frame->code->exceptions) {
				u2 cur_pc = current_frame->pc.pos();
				if (eh.start_pc <= cur_pc && cur_pc <= eh.end_pc) {
					auto type = current_frame->current_const_pool->get_class(eh.catch_type, this);
					if (type == obj->meta) {
						current_frame->pc.pos( eh.handler_pc);
						current_frame->stack->push(e);
						current_frame->interpreter(
								current_frame->current_class->name->c_str(),
								current_frame->current_method->name->c_str(),
								current_frame->current_method->discriptor->c_str()
								);
						printf("%s handle by %s.%s:%s\n", obj->meta->name->c_str(), 
								current_frame->current_class->name->c_str(), 
								current_frame->current_method->name->c_str(), 
								current_frame->current_method->discriptor->c_str());
						return true;
					}
				}
			}
			unhandle_methods.push_back(std::make_pair(current_frame->current_method, current_frame->current_pc));
		}
		pop_frame();
	}
	printf("unhandle exception %s at:\n", obj->meta->name->c_str());
	for (auto && call : unhandle_methods) {
		auto m = call.first;
		auto pc = call.second;
		auto source_attr_it = m->owner->attributes.find("SourceFile");
		auto source = m->owner->cpool->get(static_cast<source_attr*>( source_attr_it->second)->sourcefile_index);
		printf("\t%s.%s:%d ", m->owner->name->c_str(), m->name->c_str(), pc);
		if (source) {
			printf("%s", source->utf8_str->c_str());
		}else {
			printf("??????");
		}
		printf("\n");
	}
	exit(0);
	return false;
}

jvalue thread::call_native(method * m, array_stack * args)
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

	uint32_t arg_count = m->arg_types.size() + 2;//Env指针,this/class指针
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

	jreference class_or_this = m->is_static()?m->owner->mirror : args->pop<jreference>();
	args->print();
	int idx = 1;
	arg_types[1] = &ffi_type_uint32;
	arg_ptrs[1] = alloca(arg_types[1]->size);
	*(jreference*)arg_ptrs[1] = class_or_this;
	idx ++;
	for (auto & t : m->arg_types) {
		printf("put arg %d\n", t);
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
		else {
		}
		idx ++;
	}
	fflush(stdout);
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



/*
 * 任何方法调用都会经过这个
 */
jvalue thread::call(method * m, array_stack * args, bool is_interface, bool call_in_native)
{
	if (!m) return 0; 
	if (!args && current_frame) {
		args = current_frame->stack;
	}

	/*
	 * 处理interface方法
	 */
	if ((is_interface || m->is_abstract()) && args) {
		jreference this_ref = args->get<jreference>(args->top_pos() - m->arg_space - 1 - args->begin());
		m = static_cast<method*>(get_env()->lookup_method_by_object(this_ref, m->name->c_str(), m->discriptor->c_str()));
	}

	current_frame = new frame(this, m, args);
	jvalue ret = 0;

	try {
		if (m->is_native()) {
			ret = current_frame->native();
		}else {
			ret = current_frame->interpreter(m->owner->name->c_str(), m->name->c_str(), m->discriptor->c_str());
		}
	}catch (const char * e) {
		throw_exception_to_java(e);
	}
	frame * return_frame = current_frame->caller_frame;
	if (m->ret_type != T_VOID && !return_frame->current_method->is_native()) {
		if (array::slot_need2(m->ret_type) == array::slot_need<jlong>::value) {
			return_frame->stack->push(ret.j);
			log::trace("%s.%s.%s returns %ld", m->owner->name->c_str(), m->name->c_str(), m->discriptor->c_str(), return_frame->stack->top<jlong>());
		}else {
			return_frame->stack->push(ret.i);
			log::trace("%s.%s.%s returns %d", m->owner->name->c_str(), m->name->c_str(), m->discriptor->c_str(), return_frame->stack->top<jint>());
		}
	}
	pop_frame();

	return ret;
#if 0
	temp_stack->print();
	int id = 1;
	printf("with %lu args(", m->arg_types.size());
	for (auto t : m->arg_types) {
		printf("%d.%d", id ++, idx);
		if (t == T_OBJECT) {
			jreference obj = temp_stack->get<jreference>(--idx);
			object * oop = memery::ref2oop(obj);
			printf("ref(%d)",obj);
			fflush(stdout);
			if (oop) {
				printf(" %s:", oop->meta->name->c_str());
				if (oop->meta->name->equals("java/lang/String")) {
					printf("%s ", get_env()->get_utf8_string(obj).c_str());
				}else if (oop->meta->name->equals("java/lang/Class")) {
					printf("%s ", oop->meta->loader->claxx_from_mirror(obj)->name->c_str());
				}else {
					printf("%d ", obj);
				}
			}else {
				printf("null ");
			}
		}else {
			printf(" %s:", type_text[t]);
			if (t == T_FLOAT) {
				printf("%f ", temp_stack->get<jfloat>(--idx));
			}else if (t == T_DOUBLE) {
				printf("%lf ", temp_stack->get<jdouble>(idx-2));
				idx -= 2;
			}else if (t == T_LONG) {
				printf("%ld ", temp_stack->get<jlong>(idx-2));
				idx -= 2;
			}else {
				printf("%d ", temp_stack->get<jint>(--idx));
			}
		}
	}
	printf(")\n");
}	

jvalue ret = 0;


if (is_interface || m->is_abstract()) {
	jreference objref = temp_stack->top<jreference>();
	object * oop = memery::ref2oop(objref);
	m = oop->meta->lookup_method(m->name->c_str(), m->discriptor->c_str());
}

//thi指针
if (!m->is_static() && temp_stack->top<jreference>() == null) {
	fprintf(stderr, "%s 的指针空噜\n", m->owner->name->c_str());
	throw_exception_to_java("java/lang/NullPointerException"); 
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
		if (!m->is_static()) {
			new_frame->locals->put(temp_stack->pop<jreference>(),idx ++);
		}
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
		current_frame->exec(m->owner->name->c_str(), m->name->c_str());
		depth --;

		if (current_frame->exception_occured) {
			handle_exception();
		}else if (m->ret_type != T_VOID) {
			if (m->ret_type == T_LONG) {
				ret = current_frame->stack->pop<jlong>();
			}else if (m->ret_type == T_DOUBLE) {
				ret = current_frame->stack->pop<jdouble>();
			}else if (m->ret_type == T_FLOAT) {
				ret = current_frame->stack->pop<jfloat>();
			}else {
				ret = current_frame->stack->pop<jint>();
			}
		}
	}catch(const char *  e) {
		throw_exception_to_java(e);
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
#endif
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
