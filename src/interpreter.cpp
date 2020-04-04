#include "interpreter.h"
#include "class.h"
#include "classfile.h"
#include "memery.h"
#include "object.h"
#include "thread.h"
#include <cstdint>
#include <iterator>
#include <string>
#include "classloader.h"

void debug(const char * f, ...)
{
	va_list l;
	va_start(l,f);
	char buff[2048] = {0};
	vsprintf(buff, f, l);
	printf("%s", buff);
	va_end(l);
	fflush(stdout);
}

void frame::exec(const char * class_name, const char * method_name)
{
	if (current_method->is_native()) return;
	debug("codelength[%d]\n", pc.value());
	while (pc.value()) {
		debug("\e[33m[%05d]\e[0m ", pc.pos());
		u1 current_pc = pc.pos();
		u1 op = pc.get<u1>();
		debug("0x%02x ", op);
		switch (op) {
			case 0:
				debug("nop\n");
				break;
			case ACONST_NULL:
				debug("aconst_null\n");
				stack->push<jreference>(0);
				break;
			case ICONST_M1:
			case ICONST_0:
			case ICONST_1:
			case ICONST_2:
			case ICONST_3:
			case ICONST_4:
			case ICONST_5:
				{
					debug("iconst %d\n", op - ICONST_M1 - 1);
					stack->push<jint>(op - ICONST_M1 - 1);
				}
				break;
			case FCONST_0:
			case FCONST_1:
			case FCONST_2:
				{
					debug("fconst %f\n", (jfloat)(op-FCONST_0));
					stack->push<jfloat>(op - FCONST_0);
				}
				break;
			case BIPUSH:
					{
						u1 v = pc.get<u1>();
						debug("bipush %d\n", v);
						stack->push<jint>(v);
					}
				break;
			case LDC:
				{
					u1 index = pc.get<u1>();
					const_pool_item * c = current_const_pool->get(index);
					debug("current_class %s %d\n", current_class->name->c_str(), c->tag);
					if (c->tag == CONSTANT_Class ) {
						stack->push(current_const_pool->get_class(index, current_thread)->static_obj);
						break;
					}
					if (c->tag == CONSTANT_Long || c->tag == CONSTANT_Double) {
						jlong v = c->value.j;
						debug("ldc2 %d[%ld]\n", index, v);
						stack->push<jlong>(v);
					} else if (c->tag == CONSTANT_String) {
						stack->push<jreference>(current_const_pool->get_string(index, current_thread));
						debug("ldc string at %d\n", index);
					}else {
						jreference v = c->value.i;
						debug("ldc %d[%d]\n", index, v);
						stack->push<jreference>(v);
					}
				}
				break;
			case DCONST_0:
			case DCONST_1:
				{
					jdouble v = op - DCONST_0;
					debug("dconst %lf\n", v);
					stack->push(v);
				}
				break;
			case ILOAD_0:
			case ILOAD_1:
			case ILOAD_2:
			case ILOAD_3:
				{
					debug("iload %d\n", op - ILOAD_0);
					stack->push(locals->get<jint>(op-ILOAD_0));
				}
				break;
			case ALOAD:	
				{
					u1 index = pc.get<u1>();
					debug("aload %d\n", index);
					stack->push<jreference>(locals->get<jreference>(index));
				}
				break;
			case ALOAD_0:
			case ALOAD_1:
			case ALOAD_2:
			case ALOAD_3:
				debug("aload %d\n",op - ALOAD_0);
				stack->push<jreference>(locals->get<jreference>(op -ALOAD_0));
				break;
			case LSTORE:
				locals->put(stack->pop<jlong>(),pc.get<u1>());
				debug("lstore\n");
				break;
			case ASTORE:
				locals->put(stack->pop<jreference>(), pc.get<u1>());
				debug("astore\n");
				break;
			case ISTORE_0:
			case ISTORE_1:
			case ISTORE_2:
			case ISTORE_3:
				debug("istore %d\n",op - ISTORE_0);
				locals->put(stack->pop<jint>(), pc.get<u1>());
				break;
			case ASTORE_0:
			case ASTORE_1:
			case ASTORE_2:
			case ASTORE_3:
				debug("astore %d\n", op - ASTORE_0);
				locals->put<jreference>(stack->pop<jreference>(), op-ASTORE_0);
				break;
			case AASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					debug("aastore %d[%d] = %d\n", arrayref, index, value);
				}
				break;
			case CASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					debug("castore %d[%d] = %d\n", arrayref, index, value);
				}
				break;
			case POP:
				debug("pop\n");
				stack->pop<jint>();
				break;
			case POP2:
				debug("pop2\n");
				stack->pop<jlong>();
				break;
			case DUP:
				{
					debug("dup\n");
					jint top = stack->top<jint>();
					stack->push(top);
				}
				break;
			case IADD:
				{
					jint a = stack->pop<jint>();
					jint b = stack->pop<jint>();
					stack->push<jint>(a+b);
					printf("iadd %d %d\n", a, b);
				}
				break;
			case IF_ACMPEQ:
				{
					jreference a = stack->pop<jreference>();
					jreference b = stack->pop<jreference>();
					debug("if_acmpeq %u %u\n", a, b);
					if (a == b) pc_offset(pc.get<u2>());
				}
				break;
			case IF_ACMPNE:
				{
					jreference a = stack->pop<jreference>();
					jreference b = stack->pop<jreference>();
					debug("if_acmpne %u %u\n", a, b);
					if (a != b) pc_offset(pc.get<u2>());
				}
				break;
			case GOTO:
				{
					u2 offset = pc.get<u2>();
					pc_offset(offset);
					debug("goto %d\n", offset);
				}
				break;
			case GETSTATIC: 
				{
					field * f = current_const_pool->get_field(pc.get<u2>(), current_thread);
					object * oop = memery::ref2oop(f->owner->static_obj);
					jvalue v = oop->get_field(f);
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						stack->push(v.j);
					}else {
						stack->push(v.i);
					}
					debug("getstatic %s.%s\n", f->owner->name->c_str(), f->name->c_str());
				}
				break;
			case PUTSTATIC:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					object * oop = memery::ref2oop(f->owner->static_obj);
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						oop->set_field(f, stack->pop<jlong>());
					}else {
						oop->set_field(f, stack->pop<jint>());
					}
					debug("putstatic %s.%s\n", f->owner->name->c_str(), f->name->c_str());
				}
				break;
			case GETFIELD:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					jreference objref = stack->pop<jreference>();
					
					/*
					 * if (!objref) {
						claxx * nullexp = current_class->loader->load_class("java/lang/NullPointerException",current_thread);
						stack->push(memery::alloc_heap_object(nullexp));
						current_thread->handle_exception();
						return;
					}
					*/
					object * obj = memery::ref2oop(objref);
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						stack->push(obj->get_field(f).j);
					}else {
						stack->push(obj->get_field(f).i);
					}
					debug("getfield %s.%s\n on object ref %u\n", f->owner->name->c_str(), f->name->c_str(), objref);
				}
				break;
			case PUTFIELD:
				{

					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					debug("putfield %s.%s on object ref ", f->owner->name->c_str(), f->name->c_str());
					jreference objref = 0;
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						jlong v = stack->pop<jlong>();
						objref = stack->pop<jreference>();
						debug("%d\n",objref);
						object * obj = memery::ref2oop(objref);
						obj->set_field(f, v);
					}else {
						jint v = stack->pop<jint>();
						objref = stack->pop<jreference>();
						debug("%d\n",objref);
						object * obj = memery::ref2oop(objref);
						obj->set_field(f, v);
					}
				}
				break;
			case INVOKEVIRTUAL:
			case INVOKESPECIAL:
			case INVOKESTATIC:
				{
					const char * msg []  = {"virtual", "special", "static"};
					method * to_call = current_const_pool->get_method(pc.get<u2>(), current_thread);
					debug("invoke%s %s.%s\n", msg[op-INVOKEVIRTUAL],to_call->owner->name->c_str(),  to_call->name->c_str());
					if (to_call) {
						current_thread->call(to_call);
					}
				}
				break;
			case INVOKEINTERFACE:
				{
					u2 index = pc.get<u2>();
					u1 count = pc.get<u1>();
					u1 zero = pc.get<u1>();
					method * interface_method = current_const_pool->get_method(index, current_thread);
					debug("invokeinterface %s.%s [%d|%d|%d]\n ", interface_method->owner->name->c_str(), interface_method->name->c_str(), index, count, zero);
					current_thread->call(interface_method, nullptr, true);
				}
				break;
			case NEW:
				{
					claxx * meta = current_const_pool->get_class(pc.get<u2>(), current_thread);
					debug("new %s\n", meta->name->c_str());
					stack->push(memery::alloc_heap_object(meta));
				}
				break;
			case NEWARRAY:
				{
					jtype type = (jtype)pc.get<u1>();
					if (4 <= type && type <= 11) {
						const char * name[] = {"boolean", "char", "float", "double", "byte", "short", "int", "long", "object" };
					}
					stack->push(current_thread->get_env()->create_basic_array(type, stack->pop<jint>()));
				}
				break;
			case ANEWARRAY:
				{
					claxx * c = current_const_pool->get_class(pc.get<u2>(), current_thread);
					jint length = stack->pop<jint>();
					if (!c) {
						break;
					}
					claxx * ca = c->get_array_claxx(current_thread);
					stack->push(memery::alloc_heap_array(ca, length));
					debug("anewarray of %s\n", ca->name->c_str());
				}
				break;
			case ARRAYLENGTH:
				{
					jreference arrayref = stack->pop<jreference>();
					stack->push(current_thread->get_env()->array_length(arrayref));
				}
				break;
			case ATHROW:
				{
					debug("athrow\n");
					current_thread->handle_exception();
				}
				break;
			case MONITERENTER:
				{
					debug("monitor enter\n");
					stack->pop<jreference>();
				}
				break;
			case MONITEREXIT:
				{
					debug("monitor exit\n");
					stack->pop<jreference>();
				}
				break;
			case IFNULL:
				{
					u2 offset = pc.get<u2>();
					debug("ifnull %d\n", offset);
					if (stack->pop<jreference>()) pc_offset(offset);
				}
				break;
			case IFNONNULL:
				{
					jreference ref = stack->pop<jreference>();
					jshort offset = pc.get<u2>();
					debug("ifnonnull %d goto %d\n", ref, current_pc + offset);
					if (ref) pc_offset(offset);
				}
				break;
			default:
				debug("unkown instruction 0x%02x\n", op);
				fflush(stdout);
				abort();
		}
		print_stack();
		print_locals();
	}
	debug("call \e[34m[%s.%s]\e[0m exited\n", current_class->name->c_str(), current_method->name->c_str());
}

