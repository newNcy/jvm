
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
#include <thread>
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
			get_env()->throw_exception("java/lang/NullPointerException", "");
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
			abort();
			object * oop = object::from_reference(e);
			auto get_msg = oop->meta->lookup_method("getMessage","()Ljava/lang/String;");
			jreference msg_ref  = get_env()->callmethod(get_msg, e);
			logstream log(2024);
			log.printf("throw %s(%s):", oop->meta->name->c_str(), msg_ref ? get_env()->get_utf8_string(msg_ref).c_str() : "no msg");
			for (auto f : this->abrupt_frames) {
				int line = 0;
				log.printf("\tat %s.%s ", 
							f->current_class->name->c_str(), 
							f->current_method->name->c_str());


				if (f->current_method->is_native()) {
					log.printf("(native code)");
				}else {
					auto linetbs = f->code->get_attributes<line_number_table_attr>();
					int line = -1;
					for (auto tb : linetbs) {
						for (auto item = tb->line_number_table.rbegin(); item != tb->line_number_table.rend();  ++ item) {
							if (f->current_pc >= item->first) {
								line = item->second;
								break;
							}
						}
						if (line != -1) break;
					}
					log.printf("(%s):%d", f->current_const_pool->get(f->current_class->get_attributes<source_attr>()[0]->sourcefile_index)->utf8_str->c_str(), line);
				}
				fprintf(stderr, "%s", log.buf);
				delete f;
			}
			exit(0);
		}else {
			throw e;
		}
	}
	if (!current_frame) finish = true;

	return ret;
}

jreference thread::create_string(const std::string & bytes)
{
	return get_env()->create_string(bytes);
}


thread::thread(jvm * this_vm, jreference m): runtime_env(this_vm, this)
{
	if (m) {
		this->mirror = m;
	}else {
		this->mirror = java_lang_Thread.Class(this)->instantiate(this);
	}
	auto f = java_lang_Thread.Class()->lookup_field("priority");
	get_env()->set_object_field(this->mirror, f, 5);
}
	
bool thread::is_daemon()
{
	auto f = java_lang_Thread.Class()->lookup_field("daemon");
	return get_env()->get_object_field(mirror, f).z;
}
void thread::start()
{
	auto m = get_env()->lookup_method_by_object(this->mirror, "run", "()V");
	this->call(static_cast<method*>(m), mirror);
}
