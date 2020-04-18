#include "interpreter.h"
#include "attribute.h"
#include "class.h"
#include "classfile.h"
#include "memery.h"
#include "object.h"
#include "thread.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <string>
#include <sys/types.h>
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
	while (pc.value()) {
		current_pc = pc.pos();
		debug("[%s.%s::%05d] ", current_class->name->c_str(), current_method->name->c_str(), pc.pos());
		u4 op = pc.get<u1>();
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
			case LCONST_0:
			case LCONST_1:
				{
					debug("lconst %d\n", op - LCONST_0 );
					stack->push<jlong>(op - LCONST_0);
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
			case SIPUSH:
					{
						jshort v = pc.get<jshort>();
						debug("sipush %d\n", v);
						stack->push<jint>(v);
					}
				break;

			case LDC:
				{
					u1 index = pc.get<u1>();
					const_pool_item * c = current_const_pool->get(index);
					debug("current_class %s %d %d\n", current_class->name->c_str(), index, c->tag);
					if (c->tag == CONSTANT_Class ) {
						stack->push(current_const_pool->get_class(index, current_thread)->mirror);
						break;
					}
					 else if (c->tag == CONSTANT_String) {
						stack->push<jreference>(current_const_pool->get_string(index, current_thread));
						debug("ldc string at %d\n", index);
					}else {
						jreference v = c->value.i;
						debug("ldc %d[%d]\n", index, v);
						stack->push<jreference>(v);
					}
				}
				break;
			case LDC2_W:
				{
					u2 index = pc.get<u2>();
					debug("ldc2_w %d\n", index);
					const_pool_item * c = current_const_pool->get(index);
					if (c->tag == CONSTANT_Long) {
						debug("ldc2_w %d[%ld]\n", index, c->value.j);
						stack->push<jlong>(c->value.j);
					}else {
						debug("ldc2_w %d[%ld]\n", index, c->value.d);
						stack->push<jlong>(c->value.d);
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
			case ILOAD:
				{
					u1 index = pc.get<u1>();
					debug("iload %d\n", index);
					stack->push(locals->get<jint>(index));
				}
				break;
			case LLOAD:
				{
					u1 index = pc.get<u1>();
					debug("lload %d\n", index);
					stack->push(locals->get<jlong>(index));
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
			case FLOAD_0:
			case FLOAD_1:
			case FLOAD_2:
			case FLOAD_3: 
				{
					u1 index = op - FLOAD_0;
					debug("fload %d\n", index);
					stack->push(locals->get<jfloat>(index));
				}
				break;
			case ALOAD_0:
			case ALOAD_1:
			case ALOAD_2:
			case ALOAD_3:
				debug("aload %d\n",op - ALOAD_0);
				stack->push<jreference>(locals->get<jreference>(op -ALOAD_0));
				break;
			case IALOAD:
				{
					jint index = stack->pop<jint>();
					jreference ref = stack->pop<jreference>();
					jint value = current_thread->get_env()->get_array_element(ref, index);
					stack->push(value);
					debug("iaload %d[%d] -> %d\n", ref, index, value);
				}
				break;

			case AALOAD:
				{
					jint index = stack->pop<jint>();
					jreference array = stack->pop<jreference>();
					jreference e = current_thread->get_env()->get_array_element(array, index);
					debug("aaload get %d[%d] -> %u\n", array, index, e);
					stack->push(e);
				}
				break;
			case CALOAD:
				{
					jint index = stack->pop<jint>();
					jreference arr = stack->pop<jreference>();
					stack->push(current_thread->get_env()->get_array_element(arr,index).c);
					debug("caload %d[%d] %d\n", arr, index, stack->top<jreference>());
				}
				break;
			case ISTORE:
				{
					u1 index = pc.get<u1>();
					debug("istore %d\n", index);
					locals->put(stack->pop<jint>(), index); 
				}
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
				locals->put(stack->pop<jint>(),op - ISTORE_0); 
				break;
			case ASTORE_0:
			case ASTORE_1:
			case ASTORE_2:
			case ASTORE_3:
				debug("astore %d\n", op - ASTORE_0);
				locals->put<jreference>(stack->pop<jreference>(), op-ASTORE_0);
				break;
			case IASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					debug("iastore %d[%d] = %d\n", arrayref, index, value);
					current_thread->get_env()->set_array_element(arrayref, index, value);
				}
				break;
			case AASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					debug("aastore %d[%d] = %d\n", arrayref, index, value);
					current_thread->get_env()->set_array_element(arrayref, index, value);
				}
				break;
			case CASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					debug("castore %d[%d] = %d\n", arrayref, index, value);
					current_thread->get_env()->set_array_element(arrayref, index, value);
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
			case DUP_X1:
				{
					jint v1 = stack->pop<jint>();
					jint v2 = stack->pop<jint>();
					debug("dup_x1\n");
					stack->push(v1, v2, v1);
				}
				break;
			case DUP2:
				{
					debug("dup2\n");
					jlong top = stack->top<jlong>();
					stack->push(top);
				}
				break;
			case IADD:
				{
					jint b = stack->pop<jint>();
					jint a = stack->pop<jint>();
					stack->push<jint>(a+b);
					printf("iadd %d %d\n", a, b);
				}
				break;
			case LADD:
				{
					jlong b = stack->pop<jlong>();
					jlong a = stack->pop<jlong>();
					stack->push<jlong>(a+b);
					printf("ladd %ld %ld\n", a, b);
				}
				break;
			case ISUB:
				{
					jint b = stack->pop<jint>();
					jint a = stack->pop<jint>();
					stack->push<jint>(a-b);
					printf("isub %d %d\n", a, b);
				}
				break;
			case FMUL:
				{
					jfloat v2 = stack->pop<jfloat>();
					jfloat v1 = stack->pop<jfloat>();
					debug("fmul %f %f\n", v1, v2);
					stack->push(v1*v2);
				}
				break;
			case FDIV:
				{
					jfloat v2 = stack->pop<jfloat>();
					jfloat v1 = stack->pop<jfloat>();
					debug("fdiv %f %f\n", v1, v2);
					if (v2 == 0) {
						stack->push<jfloat>(0x7fc00000);
					}else {
						stack->push<jfloat>(v1/v2);
					}
				}
				break;
			case IREM:
				{
					jint b = stack->pop<jint>();
					jint a = stack->pop<jint>();
					stack->push<jint>(a-(a/b)*b);
					printf("irem %d %d\n", a, b);
				}
				break;
			case ISHL:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					jint s = v2 & 0x3f;
					jint result = v1 << s;
					debug("lshl %ld %d = %ld\n", v1, v2, result);
					stack->push(result);
				}
				break;
			case LSHL:
				{
					jint v2 = stack->pop<jint>();
					jlong v1 = stack->pop<jlong>();
					jint s = v2 & 0x3f;
					jlong result = v1 << s;
					debug("lshl %ld %d = %ld\n", v1, v2, result);
					stack->push(result);
				}
				break;
			case IOR:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					jint result = v1 | v2;
					debug("lshl %ld %d = %ld\n", v1, v2, result);
					stack->push(result);
				}
				break;
			case IUSHR:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					jint s = v2 & 0x1f;
					jint result = v1 >> s;
					if (v1 < 0) {
						result += 2<<(~s);
					}
					debug("iushr %d %d = %d\n", v1, v2, result);
					stack->push(result);
				}
				break;
			case IAND:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					jint result = v1 & v2;
					debug("land %ld %ld = %ld\n", v1, v2, result);
					stack->push(result);
				}
				break;
			case LAND:
				{
					jlong v2 = stack->pop<jlong>();
					jlong v1 = stack->pop<jlong>();
					jlong result = v1 & v2;
					debug("land %ld %ld = %ld\n", v1, v2, result);
					stack->push(result);
				}
				break;
			case IXOR:
				{
					jint a = stack->pop<jint>();
					jint b = stack->pop<jint>();
					jint result = a ^ b;
					debug("ixor %d %d = %d\n", a, b, result);
					stack->push(result);
				}
				break;
			case IINC:
				{
					u1 index = pc.get<u1>();
					char c = pc.get<char>();
					jint li = locals->get<jint>(index);
					debug("iinc index %d %d += %d\n", index, li, c);
					li += c;
					locals->put(li, index);
				}
				break;
			case I2L:
				{
					jlong v = stack->pop<jint>();
					debug("i2l %ld\n", v);
					stack->push(v);
				} 
				break;
			case I2F:
				{
					jfloat v = stack->pop<jint>();
					debug("i2f %f\n", v);
					stack->push(v);
				} 
				break;
			case F2I:
				{
					jvalue v = stack->pop<jfloat>();
					jint result = 0;
					if (v.i != 0x7fc00000) {
						result = v.f;
					}
					debug("f2i %f\n", v);
					stack->push(result);
				} 
				break;
			case I2C:
				{
					jchar v = stack->pop<jint>();
					stack->push<jint>(v);
					debug("i2c %d\n", v);
				} 
				break;
			case FCMPL:
			case FCMPG:
				{
					jint b = stack->pop<jint>();
					jint a = stack->pop<jint>();
					debug("fcmpg %d %d\n", a, b);
					if ( a > b ) stack->push<jint>(1);
					else if ( a == b ) stack->push<jint>(0);
					else stack->push<jint>(-1);
				}
				break;
			case IFEQ:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value == 0) pc.pos(current_pc + offset);
					debug("if %d == 0 goto %d\n", current_pc + offset);
				}
				break;
			case IFNE:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value != 0) pc.pos(current_pc + offset);
					debug("if %d != 0 goto %d\n", current_pc + offset);
				}
				break;
			case IFLT:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value < 0) pc.pos(current_pc + offset);
					debug("if %d != 0 goto %d\n", current_pc + offset);
				}
				break;
			case IFGE:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value >= 0) pc.pos(current_pc + offset);
					debug("if %d >= 0 goto %d\n", value, current_pc + offset);
				}
				break;
			case IFGT:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value > 0) pc.pos(current_pc + offset);
					debug("if %d >= 0 goto %d\n", value, current_pc + offset);
				}
				break;
			case IFLE:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value <= 0) pc.pos(current_pc + offset);
					debug("if %d <= 0 goto %d\n", value, current_pc + offset);
				}
				break;
			case IF_ICMPEQ:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					debug("if %u == %u goto %u+%u\n", v1, v2, current_pc, offset);
					if (v1 == v2) pc.pos(current_pc + offset);
				}
				break;

			case IF_ICMPNE:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					debug("if %u != %u goto %u+%u\n", v1, v2, current_pc, offset);
					if (v1 != v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ICMPLT:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					debug("if %u < %u goto %u+%u\n", v1, v2, current_pc, offset);
					if (v1 < v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ICMPGE:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					debug("if %u >= %u goto %u+%u\n", v1, v2, current_pc, offset);
					if (v1 >= v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ICMPGT:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					debug("if %u > %u goto %u+%u\n", v1, v2, current_pc, offset);
					if (v1 > v2) pc.pos(current_pc + offset);
				}
				break;


			case IF_ICMPLE:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					debug("if %u <= %u goto %u+%u\n", v1, v2, current_pc, offset);
					if (v1 <= v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ACMPEQ:
				{
					jreference a = stack->pop<jreference>();
					jreference b = stack->pop<jreference>();
					int16_t offset = pc.get<int16_t>();
					debug("if_acmpeq %u %u\n", a, b);
					if (a == b) pc.pos(current_pc + offset);
				}
				break;
			case IF_ACMPNE:
				{
					jreference a = stack->pop<jreference>();
					jreference b = stack->pop<jreference>();
					int16_t offset = pc.get<int16_t>();
					debug("if_acmpne %u %u\n", a, b);
					if (a != b) pc.pos(current_pc + offset);
				}
				break;
			case GOTO:
				{
					int16_t offset = pc.get<int16_t>();
					pc.pos(current_pc+offset);
					debug("goto %d\n", current_pc + offset);
				}
				break;
			case IRETURN:
				{
					debug("ireturn %d\n", stack->top<jint>());
					return;
				}
				break;
			case ARETURN:
				{
					debug("areturn %d\n", stack->top<jreference>());
					return;
				}
				break;
			case GETSTATIC: 
				{
					field * f = current_const_pool->get_field(pc.get<u2>(), current_thread);
					f->owner->loader->initialize_class(f->owner, current_thread);
					object * oop = memery::ref2oop(f->owner->static_obj);
					jvalue v = oop->get_field(f);
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						stack->push(v.j);
					}else {
						stack->push(v.i);
					}
					debug("getstatic %s.%s -> %d\n", f->owner->name->c_str(), f->name->c_str(), stack->top<jreference>());
				}
				break;
			case PUTSTATIC:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					object * oop = memery::ref2oop(f->owner->static_obj);
					jvalue v = 0; 
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						v = stack->pop<jlong>();
					}else {
						v = stack->pop<jint>();
					}
					debug("putstatic %s.%s %d\n", f->owner->name->c_str(), f->name->c_str(), v.i);
					oop->set_field(f, v);
				}
				break;
			case GETFIELD:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					jreference objref = stack->pop<jreference>();
					debug("getfield %s.%s on object ref %u\n", f->owner->name->c_str(), f->name->c_str(), objref);
					 if (!objref) {
						throw "java/lang/NullPointerException";
						//abort();
					}
					object * obj = memery::ref2oop(objref);
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						stack->push(obj->get_field(f).j);
					}else {
						stack->push(obj->get_field(f).i);
					}
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
						if (!objref) {
							abort();
						}
						debug("%d\n",objref);
						object * obj = memery::ref2oop(objref);
						obj->set_field(f, v);
					}else {
						jint v = stack->pop<jint>();
						objref = stack->pop<jreference>();
						if (!objref) {
							abort();
						}
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
					if (to_call) {
					debug("invoke%s %s.%s %s is_abstract %d\n", msg[op-INVOKEVIRTUAL],to_call->owner->name->c_str(),  to_call->name->c_str(), to_call->discriptor->c_str(), to_call->is_abstract());
						if (to_call->is_static()) {
							if (to_call->owner->state < INITED) to_call->owner->loader->initialize_class(to_call->owner, current_thread);
						}
						current_thread->call(to_call);
					}else {
					}
				}
				break;
			case INVOKEINTERFACE:
				{
					u2 index = pc.get<u2>();
					u1 count = pc.get<u1>();
					u1 zero = pc.get<u1>();
					method * interface_method = current_const_pool->get_method(index, current_thread);
					debug("invokeinterface %s.%s %s [%d|%d|%d]\n ", interface_method->owner->name->c_str(), interface_method->name->c_str(), interface_method->discriptor->c_str(), index, count, zero);
					current_thread->call(interface_method, nullptr, true);
				}
				break;
			case NEW:
				{
					u2 index = pc.get<u2>();
					const_pool_item * sym = current_const_pool->get(index);
					debug("new %s\n", sym->sym_class->name->c_str());
					claxx * meta = current_const_pool->get_class(index, current_thread);
					if (meta->state < INITED) meta->loader->initialize_class(meta, current_thread);
					stack->push(memery::alloc_heap_object(meta));
				}
				break;
			case NEWARRAY:
				{
					jtype type = (jtype)pc.get<u1>();
					debug("new array of %s\n", type_text[type]);
					stack->push(current_thread->get_env()->create_basic_array(type, stack->pop<jint>()));
				}
				break;
			case ANEWARRAY:
				{
					claxx * c = current_const_pool->get_class(pc.get<u2>(), current_thread);
					if (c->state < INITED) c->loader->initialize_class(c, current_thread);
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
					debug("array length of %d is %d\n", arrayref, stack->top<jint>());
				}
				break;
			case ATHROW:
				{
					debug("athrow\n");
					exception_occured = true;
					return;
				}
				break;
			case CHECKCAST:
				{
					u2 index = pc.get<u2>();
					claxx * cls = current_const_pool->get_class(index, current_thread);
					jreference ref = stack->top<jreference>();
					object * obj = memery::ref2oop(ref);
					debug("checkcast %d to %s\n", stack->top<jreference>(), cls->name->c_str());
					if (obj && !obj->meta->check_cast(cls)) {
						throw std::string("java/lang/ClassCastException");
					}
				}
				break;
			case INSTANCEOF:
				{
					u2 index = pc.get<u2>(); 
					jreference ref = stack->pop<jreference>();
					if (ref == null) {
						stack->push<jint>(0);
						break;
					}
					claxx * cls = current_const_pool->get_class(index, current_thread);
					debug("%d is instance of %s\n", ref, cls->name->c_str());
					jint result = memery::ref2oop(ref)->is_instance(cls);
					stack->push(result);
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
					int16_t offset = pc.get<int16_t>();
					debug("ifnull %d\n", current_pc + offset);
					if (!stack->pop<jreference>()) pc.pos(current_pc+offset);
				}
				break;
			case IFNONNULL:
				{
					jreference ref = stack->pop<jreference>();
					jshort offset = pc.get<u2>();
					debug("ifnonnull %d goto %d\n", ref, current_pc + offset);
					if (ref) pc.pos(current_pc + offset);
				}
				break;
			default:
				debug("unkown instruction 0x%02x\n", op);
				fflush(stdout);
				abort();
		}
		print_stack();
		print_locals();
		fflush(stdout);
	}
}

