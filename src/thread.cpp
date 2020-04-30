
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
	log::debug("%x deleted", current_frame);
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
#if 1
						printf("%s handle by %s.%s:%s\n", obj->meta->name->c_str(), 
								current_frame->current_class->name->c_str(), 
								current_frame->current_method->name->c_str(), 
								current_frame->current_method->discriptor->c_str());
#endif
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

/*
 * 任何方法调用都会经过这个
 */
jvalue thread::call(method * m, array_stack * args)
{
	if (!m) return 0; 
	if (!args && current_frame) {
		args = current_frame->stack;
	}

	/*
	 * 处理interface方法
	 */
#if 0
	if ((is_interface || m->is_abstract()) && args) {
		jreference this_ref = args->get<jreference>(args->top_pos() - m->arg_space - 1 - args->begin());
		m = static_cast<method*>(get_env()->lookup_method_by_object(this_ref, m->name->c_str(), m->discriptor->c_str()));
	}
#endif

	current_frame = new frame(this, m, args);

	//check this
	if (!m->is_static()) {
		if (current_frame->locals->get<jreference>(0) == null) {
			pop_frame();
			throw_exception_to_java("java/lang/NullPointerException");
		}
	}
	jvalue ret = 0;
	try {
		if (m->is_native()) {
			ret = current_frame->native();
		}else {
			ret = current_frame->interpreter(m->owner->name->c_str(), m->name->c_str(), m->discriptor->c_str());
		}
		frame * return_frame = current_frame->caller_frame;
		if (return_frame) {
			if (m->ret_type != T_VOID && !return_frame->current_method->is_native()) {
				if (array::slot_need2(m->ret_type) == array::slot_need<jlong>::value) {
					return_frame->stack->push(ret.j);
				}else {
				return_frame->stack->push(ret.i);
				}
			}
		}
		delete current_frame;
		current_frame = return_frame;
	}catch (jreference e) {
		abrupt_frames.push_back(current_frame); //收起来
		current_frame = current_frame->caller_frame;
		if (!current_frame) {
			object * oop = object::from_reference(e);
			auto msg = oop->meta->lookup_field("detailMessage");
			log::debug("throw %s(%s):", oop->meta->name->c_str(), get_env()->get_utf8_string(get_env()->get_object_field(e, msg)).c_str());
			for (auto f : this->abrupt_frames) {
				log::debug("\tat %s.%s", f->current_class->name->c_str(), f->current_method->name->c_str());
				delete f;
			}
			exit(0);
		}else {
			throw e;
		}
	}

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
