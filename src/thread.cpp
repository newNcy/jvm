
#include "thread.h"

frame::frame(frame * caller, method * to_call):caller_frame(caller), current_method(to_call), current_class(to_call->owner)
{
	if (!current_class) return;
	current_const_pool = current_class->cpool;

	auto code_it = current_method->attributes.find("Code");
	if (code_it == current_method->attributes.end()) return;
	code = code_it->second;
	
	pc.set_buf((const char*)code->code, code->code_length);
	stack = new operand_stack(code->max_stacks);
	locals = new any_array(code->max_locals);

	gettimeofday(&start_time, nullptr);

	idx = 0;
	int argid = 1;
	for (auto type : m->arg_types) {
		switch (type) {
			case LONG:
				{
					jlong v = stack->pop<jlong>();
					printf("arg%d long:%ld\n", argid++, v);
					locals->put<jlong>(v, idx);
					idx += 2;
				}
				break;
			case DOUBLE:
				{
					jdouble v = stack->pop<jdouble>();
					printf("arg%d double:%lf\n", argid++, v);
					locals->put<jdouble>(v, idx);
					idx += 2;
				}
				break;
			case FLOAT:
				{
					jfloat v = temp.pop<jfloat>();
					printf("arg%d float:%f\n", argid++, v);
					current_frame->locals->put(v, idx);
					idx ++;
				}
				break;
			default:
				{
					jint v = temp.pop<jint>();
					printf("arg%d int:%d\n", argid++, v);
					current_frame->locals->put<jint>(v, idx);
					idx ++;
				}
		}
	}
}

~frame::frame()
{
}

void thread::push_frame(method * m)
{
	if (!m) return;
	frame * new_frame = new frame(current_frame, m);
	if (!current_frame) return;
	//arguments
	int idx = m->arg_types.size() -1 ;
	for (int i = idx; i >= 0; i --) {
		switch (m->arg_types[i]) {
			case LONG:
			case DOUBLE:
				new_frame->stack->push<jlong>(current_frame->stack->pop<jlong>());
				break;
			default:
				new_frame->stack->push<jint>(current_frame->stack->pop<jint>());
				break;

		}
	}
}

void thread::pop_frame()
{
	timeval end_time;
	gettimeofday(&end_time, nullptr);
	printf("call end\n");
	timeval & start_time = current_frame->start_time;
	frame * next = current_frame->caller_frame;
	current_frame->destroy();

	memery::dealloc_meta<frame>(current_frame);
	current_frame = next;
	if (!current_frame) {
		return;
	}
	current_method = current_frame->current_method;
	current_class = current_method->owner;
	current_const_pool = current_class->cpool;
	code = &current_frame->code;
}
	
bool thread::handle_exception()
{
		std::stack<method*> unhandle_methods;
		jreference e = current_stack.top<jreference>();
		object * obj = memery::ref2oop(e);
		for (;current_frame;) {
			for (exception & e : current_frame->code_source->exceptions) {
				code = &current_frame->code;
				u2 cur_pc = code->rd;
				if (e.start_pc <= cur_pc && cur_pc <= e.end_pc) {
					code->rd = e.handler_pc;
					printf("%s handle by %s.%s:%s\n", obj->meta->name->c_str(), current_class->name->c_str(), current_method->name->c_str(), current_method->discriptor->c_str());
					return true;
				}
			}
			unhandle_methods.push(current_frame->current_method);
			pop_frame();
		}
		print_stack();
		printf("unhandle exception %s at:\n", obj->meta->name->c_str());
		while (!unhandle_methods.empty()) {
			auto m = unhandle_methods.top();
			unhandle_methods.pop();
			printf("in %s\n", m->name->c_str());
		}
		return false;
	}

