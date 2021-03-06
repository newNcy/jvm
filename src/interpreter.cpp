#include "interpreter.h"
#include "attribute.h"
#include "class.h"
#include "classfile.h"
#include "frame.h"
#include "memery.h"
#include "object.h"
#include "thread.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include "classloader.h"
#include "log.h"

jvalue frame::interpreter(const char * a, const char * b, const char * c)
{
	if (current_method->is_native()) return 0;
	while (pc.value()) {
		current_pc = pc.pos();
		u4 op = pc.get<u1>();
		try {
		switch (op) {
			case 0:
				break;
			case ACONST_NULL:
				stack->push<jreference>(0);
				log::bytecode(this, op, "aconst null");
				break;
			case ICONST_M1:
			case ICONST_0:
			case ICONST_1:
			case ICONST_2:
			case ICONST_3:
			case ICONST_4:
			case ICONST_5:
				{
					stack->push<jint>(op - ICONST_M1 - 1);
					log::bytecode(this, op, "iconst", op-ICONST_M1-1);
				}
				break;
			case LCONST_0:
			case LCONST_1:
				{
					stack->push<jlong>(op - LCONST_0);
					log::bytecode(this, op, "lconst", op - LCONST_0 );
				}
				break;
			case FCONST_0:
			case FCONST_1:
			case FCONST_2:
				{
					stack->push<jfloat>(op - FCONST_0);
					log::bytecode(this, op, "fconst", (jfloat)(op-FCONST_0));
				}
				break;
			case BIPUSH:
					{
						jint v = pc.get<char>();
						stack->push(v);
						log::bytecode(this, op, "bipush ", v);
					}
				break;
			case SIPUSH:
					{
						jshort v = pc.get<jshort>();
						stack->push<jint>(v);
						log::bytecode(this, op, "sipush", v);
					}
				break;
			case LDC:
			case LDC_W:
			case LDC2_W:
				{
					u2 index  = 0 ;
					if (op == LDC_W || op == LDC2_W) index = pc.get<u2>();
					else index = pc.get<u1>();
					const_pool_item * c = current_const_pool->get(index);

					if (c->tag == CONSTANT_Class ) {
						claxx * meta = current_const_pool->get_class(index, current_thread);
						stack->push(meta->mirror);
						 log::bytecode(this, op, "ldc class", meta->name->c_str(), " from", index);
					}else if (c->tag == CONSTANT_String) {
						 jreference str = current_const_pool->get_string(index, current_thread);
						 stack->push(str);
						 log::bytecode(this, op, "ldc string \"", current_thread->get_env()->get_utf8_string(str), "\" from", index);
					}else if (op == LDC2_W) {
						stack->push<jlong>(c->value.j);
						if (c->tag == CONSTANT_Long) {
							log::bytecode(this, op, "ldc long", c->value.j, "from", index);
						}else {
							log::bytecode(this, op, "ldc double", c->value.d, "from", index);
						}
					}else { 
						stack->push(c->value.i);
						log::bytecode(this, op, "ldc", c->value.i, "from", index);
					}
				}
				break;
			case DCONST_0:
			case DCONST_1:
				{
					jdouble v = op - DCONST_0;
					stack->push(v);
					log::bytecode(this, op, "dconst", v);
				}
				break;
			case ILOAD:
				{
					jint index = pc.get<u1>();
					stack->push(locals->get<jint>(index));
					log::bytecode(this, op, "iload", index);
				}
				break;
			case LLOAD:
				{
					jint index = pc.get<u1>();
					stack->push(locals->get<jlong>(index));
					log::bytecode(this, op, "lload", index);
				}
				break;
			case FLOAD:
				{
					jint index = pc.get<u1>();
					stack->push(locals->get<jfloat>(index));
					log::bytecode(this, op, "fload", index);
				}
				break;
			case ILOAD_0:
			case ILOAD_1:
			case ILOAD_2:
			case ILOAD_3:
				{
					stack->push(locals->get<jint>(op-ILOAD_0));
					log::bytecode(this, op, "iload", op - ILOAD_0);
				}
				break;
			case ALOAD:	
				{
					jint index = pc.get<u1>();
					jreference obj = locals->get<jreference>(index);
					stack->push(obj);
					log::bytecode(this, op, "aload", index);
				}
				break;
			case LLOAD_0:
			case LLOAD_1:
			case LLOAD_2:
			case LLOAD_3:
				{
					jint index = op - LLOAD_0;
					log::bytecode(this, op, "lload", index);
					stack->push(locals->get<jlong>(index));
				}
				break;
			case FLOAD_0:
			case FLOAD_1:
			case FLOAD_2:
			case FLOAD_3: 
				{
					jint index = op - FLOAD_0;
					stack->push(locals->get<jfloat>(index));
					log::bytecode(this, op, "fload ", locals->get<jfloat>(index), "from", index);
				}
				break;
			case ALOAD_0:
			case ALOAD_1:
			case ALOAD_2:
			case ALOAD_3: 
				{
					jint index = op - ALOAD_0;
					stack->push(locals->get<jreference>(index));
					log::bytecode(this, op, "aload", index);
				}
				break;
			case IALOAD:
				{
					jint index = stack->pop<jint>();
					jreference ref = stack->pop<jreference>();
					jint value = current_thread->get_env()->get_array_element(ref, index);
					stack->push(value);
					log::bytecode(this, op, "iaload", value, "from", ref, '[', index, ']');
				}
				break;
			case AALOAD:
				{
					jint index = stack->pop<jint>();
					jreference array = stack->pop<jreference>();
					jreference e = current_thread->get_env()->get_array_element(array, index);
					log::bytecode(this, op, "aaload", e, "from", array, '[', index, ']');
					stack->push(e);
				}
				break;
			case BALOAD:
				{
					jint index = stack->pop<jint>();
					jreference array = stack->pop<jreference>();
					jboolean e = current_thread->get_env()->get_array_element(array, index);
					log::bytecode(this, op, "aaload", e, "from", array, '[', index, ']');
					stack->push(e);
				}
				break;
			case CALOAD:
				{
					jint index = stack->pop<jint>();
					jreference arr = stack->pop<jreference>();
					stack->push(current_thread->get_env()->get_array_element(arr,index).c);
					log::bytecode(this,op, "caload", stack->top<jreference>(), "from", arr, '[', index, ']');
				}
				break;
			case ISTORE:
				{
					jint index = pc.get<u1>();
					locals->put(stack->pop<jint>(), index); 
					log::bytecode(this, op, "istore", index);
				}
				break;
			case LSTORE: 
				{
					jint index = pc.get<u1>();
					jlong obj = stack->pop<jlong>();
					locals->put(obj, index);
					log::bytecode(this, op, "lstore", obj, "to", index);
				}
				break;
			case FSTORE: 
				{
					jint index = pc.get<u1>();
					jfloat obj = stack->pop<jfloat>();
					locals->put(obj, index);
					log::bytecode(this, op, "fstore", obj, "to", index);
				}
				break;
			case ASTORE:
				{
					jint index = pc.get<u1>();
					jreference obj = stack->pop<jreference>();
					locals->put(obj, index);
					log::bytecode(this, op, "astore", obj, "to", index);
				}
				break;
			case ISTORE_0:
			case ISTORE_1:
			case ISTORE_2:
			case ISTORE_3:
				locals->put(stack->pop<jint>(), op - ISTORE_0); 
				log::bytecode(this, op, "istore",op - ISTORE_0);
				break;
			case LSTORE_0:
			case LSTORE_1:
			case LSTORE_2:
			case LSTORE_3:
				locals->put(stack->pop<jlong>(), op - LSTORE_0); 
				log::bytecode(this, op, "lstore",op - LSTORE_0);
				break;
			case FSTORE_0:
			case FSTORE_1:
			case FSTORE_2:
			case FSTORE_3:
				{
					jfloat value = stack->pop<jfloat>();
					locals->put(value, op - FSTORE_0);
					log::bytecode(this, op, "fstore", value, "to" ,op - FSTORE_0, value);
				}
				break;
			case ASTORE_0:
			case ASTORE_1:
			case ASTORE_2:
			case ASTORE_3:
				locals->put<jreference>(stack->pop<jreference>(), op-ASTORE_0);
				log::bytecode(this, op, "astore", op - ASTORE_0);
				break;
			case IASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					log::bytecode(this, op, "iastore", value, "to", arrayref, '[', index, ']');
					current_thread->get_env()->set_array_element(arrayref, index, value);
				}
				break;
			case LASTORE:
				{
					jlong value = stack->pop<jlong>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					log::bytecode(this, op, "lastore", value, "to", arrayref, '[', index, ']');
					current_thread->get_env()->set_array_element(arrayref, index, value);
				}
				break;
			case AASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					log::bytecode(this, op, "aastore", value, "to", arrayref, '[', index, ']');
					current_thread->get_env()->set_array_element(arrayref, index, value);
				}
				break;
			case BASTORE:
				{
					jreference arrayref;
					jint index, value;
					stack->pop(arrayref, index, value);
					log::bytecode(this, op, "bastore", value, "to", arrayref, '[', index, ']');
					current_thread->get_env()->set_array_element(arrayref, index, value);
				}
				break;
			case CASTORE:
				{
					jint value = stack->pop<jint>();
					jint index = stack->pop<jint>();
					jreference arrayref = stack->pop<jreference>();
					log::bytecode(this, op, "catore", value, "to", arrayref,"[",index,"]");
					current_thread->get_env()->set_array_element(arrayref, index, value);
				}
				break;
			case POP:
				stack->pop<jint>();
				log::bytecode(this, op, "pop");
				break;
			case POP2:
				stack->pop<jlong>();
				log::bytecode(this, op, "pop2");
				break;
			case DUP:
				{
					jint top = stack->top<jint>();
					log::bytecode(this, op, "dup");
					stack->push(top);
				}
				break;
			case DUP_X1:
				{
					jint v1 = stack->pop<jint>();
					jint v2 = stack->pop<jint>();
					log::bytecode(this, op, "dup_x1", v2, v1);
					stack->push(v1, v2, v1);
				}
				break;
			case DUP2:
				{
					jlong top = stack->top<jlong>();
					stack->push(top);
					log::bytecode(this, op, "dup2", top);
				}
				break;
			case DUP2_X1:
				{
					jint v3, v2, v1;
					stack->pop(v3, v2, v1);
					stack->push(v2, v1, v3, v2, v1);
					log::bytecode(this, op, "dup2_x1", v3, v2, v1);
				}
				break;
			case IADD:
				{
					jint b = stack->pop<jint>();
					jint a = stack->pop<jint>();
					stack->push<jint>(a+b);
					log::bytecode(this, op, "iadd", a, b);
				}
				break;
			case LADD:
				{
					jlong b = stack->pop<jlong>();
					jlong a = stack->pop<jlong>();
					stack->push<jlong>(a+b);
					log::bytecode(this, op, "ladd", a, b);
				}
				break;
			case DADD:
				{
					jdouble a, b;
					stack->pop(a, b);
					stack->push(a+b);
					log::bytecode(this, op, "dadd", a, b);
				}
				break;
			case ISUB:
				{
					jint b = stack->pop<jint>();
					jint a = stack->pop<jint>();
					stack->push<jint>(a-b);
					log::bytecode(this, op,"isub", a, b);
				}
				break;
			case IMUL:
				{
					jint v1, v2;
					stack->pop(v1, v2);
					stack->push(v1*v2);
					log::bytecode(this, op, "imul", v1, v2);
				}
				break;
			case LMUL:
				{
					jlong v1, v2;
					stack->pop(v1, v2);
					stack->push(v1*v2);
					log::bytecode(this, op, "lmul", v1, v2);
				}
				break;
			case FMUL:
				{
					jfloat v2 = stack->pop<jfloat>();
					jfloat v1 = stack->pop<jfloat>();
					stack->push(v1*v2);
					log::bytecode(this, op, "fmul", v1, v2);
				}
				break;
			case IDIV:
				{
					jint v1,v2;
					stack->pop(v1, v2);
					log::bytecode(this, op, "idiv", v1, v2);
					if (v2 == 0) {
						stack->push<jint>(0);
					}else {
						stack->push<jint>(v1/v2);
					}
				}
				break;
			case FDIV:
				{
					jfloat v2 = stack->pop<jfloat>();
					jfloat v1 = stack->pop<jfloat>();
					log::bytecode(this, op, "fdiv", v1, v2);
					if (v2 == 0) {
						stack->push<>(0x7fc00000);
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
					log::bytecode(this, op, "irem", a, b);
				}
				break;
			case INEG:
				{
					jint b = stack->pop<jint>();
					stack->push<jint>(-b);
					log::bytecode(this, op, "ineg", b);
				}
				break;
			case ISHL:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					jint s = v2 & 0x3f;
					jint result = v1 << s;
					log::bytecode(this,	op, "lshl", v1, s, result);
					stack->push(result);
				}
				break;
			case LSHL:
				{
					jint v2 = stack->pop<jint>();
					jlong v1 = stack->pop<jlong>();
					jint s = v2 & 0x3f;
					jlong result = v1 << s;
					log::bytecode(this, op, "lshl", v1, s, result);
					stack->push(result);
				}
				break;
			case ISHR:
				{
					jint v2 = stack->pop<jint>();
					jlong v1 = stack->pop<jint>();
					jint s = v2 & 0x3f;
					jint result = v1 >> s;
					log::bytecode(this, op, "ishr", v1, s, result);
					stack->push(result);
				}
				break;
			case IOR:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					jint result = v1 | v2;
					log::bytecode(this, op, "lshl", v1, v2, result);
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
					log::bytecode(this, op, "iushr", v1, s, result);
					stack->push(result);
				}
				break;
			case IAND:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					jint result = v1 & v2;
					log::bytecode(this, op, "land", v1, v2);
					stack->push(result);
				}
				break;
			case LAND:
				{
					jlong v2 = stack->pop<jlong>();
					jlong v1 = stack->pop<jlong>();
					jlong result = v1 & v2;
					log::bytecode(this, op, "land", v1, v2);
					stack->push(result);
				}
				break;
			case IXOR:
				{
					jint a = stack->pop<jint>();
					jint b = stack->pop<jint>();
					jint result = a ^ b;
					log::bytecode(this, op, "ixor", a, b, result);
					stack->push(result);
				}
				break;
			case IINC:
				{
					jint index = pc.get<u1>();
					char c = pc.get<char>();
					jint li = locals->get<jint>(index);
					log::bytecode(this, op, "iinc", index);
					li += c;
					locals->put(li, index);
				}
				break;
			case I2L:
				{
					jint v = stack->pop<jint>();
					stack->push<jlong>(v);
					log::bytecode(this, op, "i2l", v);
				} 
				break;
			case I2F:
				{
					jfloat v = stack->pop<jint>();
					stack->push(v);
					log::bytecode(this, op, "i2f", v);
				} 
				break;
			case L2I:
				{
					jint v = stack->pop<jlong>();
					stack->push(v);
					log::bytecode(this, op, "l2i", v);
				} 
				break;
			case L2F:
				{
					jfloat v = stack->pop<jlong>();
					stack->push(v);
					log::bytecode(this, op, "l2f", v);
				} 
				break;
			case F2I:
				{
					jvalue v = stack->pop<jfloat>();
					jint result = 0;
					if (v.i != 0x7fc00000) { //NaN
						result = v.f;
					}
					stack->push(result);
					log::bytecode(this, op, "f2i", v.f, result);
				} 
				break;
			case F2D:
				{
					jdouble v = stack->pop<jfloat>();
					stack->push(v);
					log::bytecode(this, op, "f2d", v);
				}
				break;
			case D2L:
				{
					jlong v = stack->pop<jdouble>();
					stack->push(v);
					log::bytecode(this, op, "d2l", v);
				}
				break;
			case I2B:
				{
					jbyte v = stack->pop<jint>();
					stack->push<jint>(v);
					log::bytecode(this, op, "i2b", v);
				} 
				break;
			case I2C:
				{
					jint v = stack->pop<jint>();
					stack->push<jint>(v);
					log::bytecode(this, op, "i2c", v);
				} 
				break;
			case LCMPL: 
				{
					jlong v1, v2;
					stack->pop(v1, v2);

					if (v1 < v2) {
						stack->push(-1);
					}else if (v1 == v2) {
						stack->push(0);
					}else {
						stack->push(1);
					}
					log::bytecode(this, op, "lcmp", v1, v2);
				}
				break;
			case FCMPL:
			case FCMPG:
				{
					const char * msg [] = {"fcmpl", "fcmpg"};
					jfloat b = stack->pop<jfloat>();
					jfloat a = stack->pop<jfloat>();

					if ( a > b ) stack->push<jint>(1);
					else if ( a < b ) stack->push<jint>(-1);
					else stack->push<jint>(0);
					log::bytecode(this, op, msg[op-FCMPL], a, b);
				}
				break;
			case IFEQ:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value == 0) pc.pos(current_pc + offset);
					log::bytecode(this, op, "if", value, "== 0 goto", current_pc + offset);
				}
				break;
			case IFNE:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value != 0) pc.pos(current_pc + offset);
					log::bytecode(this, op, "if", value, "!= 0 goto", current_pc + offset);
				}
				break;
			case IFLT:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value < 0) pc.pos(current_pc + offset);
					log::bytecode(this, op, "if", value, "< 0 goto", current_pc + offset);
				}
				break;
			case IFGE:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value >= 0) pc.pos(current_pc + offset);
					log::bytecode(this, op, "if", value, ">= 0 goto", current_pc + offset);
				}
				break;
			case IFGT:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value > 0) pc.pos(current_pc + offset);
					log::bytecode(this, op, "if", value, "> 0 goto", current_pc + offset);
				}
				break;
			case IFLE:
				{
					jint value = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					if (value <= 0) pc.pos(current_pc + offset);
					log::bytecode(this, op, "if", value, "<= 0 goto", current_pc + offset);
				}
				break;
			case IF_ICMPEQ:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", v1, "==", v2,"goto", current_pc + offset);
					if (v1 == v2) pc.pos(current_pc + offset);
				}
				break;

			case IF_ICMPNE:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", v1, "!=", v2, "goto", current_pc + offset);
					if (v1 != v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ICMPLT:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", v1, '<', v2, "goto", current_pc + offset);
					if (v1 < v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ICMPGE:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", v1,  ">=", v2, "goto", current_pc + offset);
					if (v1 >= v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ICMPGT:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", v1, '>', v2, "goto", current_pc + offset);
					if (v1 > v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ICMPLE:
				{
					jint v2 = stack->pop<jint>();
					jint v1 = stack->pop<jint>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", v1,  "<=", v2, "goto", current_pc + offset);
					if (v1 <= v2) pc.pos(current_pc + offset);
				}
				break;
			case IF_ACMPEQ:
				{
					jreference a = stack->pop<jreference>();
					jreference b = stack->pop<jreference>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", a, "==", b, "goto", current_pc + offset);
					if (a == b) pc.pos(current_pc + offset);
				}
				break;
			case IF_ACMPNE:
				{
					jreference a = stack->pop<jreference>();
					jreference b = stack->pop<jreference>();
					int16_t offset = pc.get<int16_t>();
					log::bytecode(this, op, "if", a, "!=", b, "goto", current_pc + offset);
					if (a != b) pc.pos(current_pc + offset);
				}
				break;
			case GOTO:
				{
					int16_t offset = pc.get<int16_t>();
					pc.pos(current_pc+offset);
					log::bytecode(this, op, "goto", current_pc + offset);
				}
				break;
			case TABLESWITCH:
				{
					jint index = stack->pop<jint>();

					/*
					 * 同lookupswitch
					 */
					if (pc.pos() % 4 != 0) {
						pc.pos_offset(4-pc.pos()%4);
					}
					
					jint default_value = pc.get<jint>();
					jint low = pc.get<jint>();
					jint hight = pc.get<jint>();

					log::bytecode(this, op, "switch", index, "in", low , "to", hight);
					if (index < low || hight < index) {
						log::bytecode(this, op, "default", current_pc + default_value);
						pc.pos(current_pc + default_value);
					}else {
						pc.pos_offset((index-low)*4);
						jint go = pc.get<jint>();
						log::bytecode(this, op , "case", index, "goto", current_pc + go);
						pc.pos(current_pc + go);
					}
				}
				break;
			case LOOKUPSWITCH:
				{
					jint key = stack->pop<jint>();
					
					/*
					 * 让 default_value 的第一个字节地址是4的整数
					 */
					if (pc.pos() % 4 != 0) {
						pc.pos_offset(4-pc.pos()%4);
					}

					jint default_value = pc.get<jint>();
					jint npairs = pc.get<jint>();
					log::bytecode(this, op, "switch", key, "in", npairs, "pairs");
#if 1
					jint start_address = pc.pos();
					//case是排好序的，可以折半搜索
					jint start = 0, end = npairs-1;
					bool matched = false;
					while (start <= end) {
						jint mid = (start + end)/2;
						pc.pos(start_address + mid * 8);
						jint match = pc.get<jint>();
						jint pos = pc.get<jint>();
						log::bytecode(this, op, "case", mid, match, ":", current_pc + pos);
						if (match == key) {
							pc.pos(current_pc + pos);
							matched = true;
							break;
						}else if (match > key) {
							end = mid - 1;
						}else {
							start = mid + 1;
						}
					}
#else 
					for (int i = 0; i < npairs; i ++) {
						jint begin = pc.pos();
						jint match = pc.get<jint>();
						jint offset = pc.get<jint>();
						log::bytecode(this, op, "case", begin, match, "goto" , current_pc + offset);
						if (match == key ) {
							pc.pos(current_pc + offset);
							matched = true;
							break;
						}
					}
#endif
					//getchar();
					if (!matched) {
						log::bytecode(this, op, "default", ':',  current_pc + default_value);
						pc.pos(current_pc + default_value);
					}
				}
				break;
			case IRETURN:
				{
					log::bytecode(this, op, "ireturn", stack->top<jint>());
					return stack->pop<jint>();
				}
				break;
			case LRETURN:
				{
					log::bytecode(this, op, "lreturn", stack->top<jlong>());
					return stack->pop<jlong>();
				}
				break;
			case FRETURN:
				{
					log::bytecode(this, op, "freturn", stack->top<jfloat>());
					return stack->pop<jfloat>();
				}
				break;
			case DRETURN:
				{
					log::bytecode(this, op, "dreturn", stack->top<jdouble>());
					return stack->pop<jdouble>();
				}
				break;
			case ARETURN:
				{
					log::bytecode(this, op, "areturn", stack->top<jreference>());
					return stack->pop<jreference>();
				}
				break;
			case RETURN:
				{
					log::bytecode(this, op, "return");
					return 0;
				}
				break;
			case GETSTATIC: 
				{
					field * f = current_const_pool->get_field(pc.get<u2>(), current_thread);
					f->owner->loader->initialize_class(f->owner, current_thread);
					if (!f->owner->static_obj) {
						f->owner->loader->initialize_class(f->owner, current_thread);
					}
					object * oop = memery::ref2oop(f->owner->static_obj);
					jvalue v = oop->get_field(f);
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						stack->push(v.j);
					}else {
						stack->push(v.i);
					}
					log::bytecode(this, op, "getstatic", f->owner->name->c_str(), f->name->c_str(), stack->top<jreference>());
				}
				break;
			case PUTSTATIC:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					f->owner->loader->initialize_class(f->owner, current_thread);
					object * oop = memery::ref2oop(f->owner->static_obj);
					jvalue v = 0; 
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						v = stack->pop<jlong>();
					}else {
						v = stack->pop<jint>();
					}
					log::bytecode(this, op, "putstatic", f->owner->name->c_str(), f->name->c_str(), v.i); 
					oop->set_field(f, v);
				}
				break;
			case GETFIELD:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					jreference objref = stack->pop<jreference>();
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
					log::bytecode(this, op,"getfield", f->owner->name->c_str(), f->name->c_str(), f->discriptor->c_str(), f->offset, "on object", objref);
				}
				break;
			case PUTFIELD:
				{

					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					//log::debug("putfield %s.%s on object ref ", f->owner->name->c_str(), f->name->c_str());
					log::bytecode(this, op, "putfield", f->owner->name->c_str(), f->name->c_str(), f->discriptor->c_str());
					jreference objref = 0;
					if (f->type == T_LONG || f->type == T_DOUBLE) {
						jlong v = stack->pop<jlong>();
						objref = stack->pop<jreference>();
						if (!objref) {
							abort();
						}
						object * obj = memery::ref2oop(objref);
						obj->set_field(f, v);
					}else {
						jint v = stack->pop<jint>();
						objref = stack->pop<jreference>();
						if (!objref) {
							abort();
						}
						object * obj = memery::ref2oop(objref);
						obj->set_field(f, v);
					}
				}
				break;
			case INVOKEVIRTUAL:
			case INVOKESPECIAL:
			case INVOKESTATIC:
			case INVOKEINTERFACE:
				{
					try {
					const char * msg []  = {"invokevirtual", "invokespecial", "invokestatic", "invokeinterface"};
					method * target_method = current_const_pool->get_method(pc.get<u2>(), current_thread);
					log::bytecode(this, op, msg[op-INVOKEVIRTUAL], target_method->owner->name->c_str(), target_method->name->c_str(), target_method->discriptor->c_str());

					if (op == INVOKESTATIC) {
						if (target_method->owner->state < INITED) {
							target_method->owner->loader->initialize_class(target_method->owner, current_thread);
						}
					}else if (op == INVOKESPECIAL) {
					}else {
						if (op == INVOKEINTERFACE) {
							jint count = pc.get<u1>();
							jint zero = pc.get<u1>();
						}
						jreference thix = stack->get<jreference>(stack->top_pos() - target_method->arg_space - 1 - stack->begin());
						if (!thix) {
							throw "java/lang/NullPointerException";
						}

						method * real_method = static_cast<method*>(current_thread->get_env()->lookup_method_by_object(thix, target_method->name->c_str(), target_method->discriptor->c_str()));

						target_method = real_method;
					}
					current_thread->call(target_method);
					}catch (jreference e) {
						handle_exception(e);
					}
				}
				break;
			case NEW:
				{
					u2 index = pc.get<u2>();
					const_pool_item * sym = current_const_pool->get(index);
					claxx * meta = current_const_pool->get_class(index, current_thread);
					if (meta->state < INITED) meta->loader->initialize_class(meta, current_thread);
					stack->push(meta->instantiate(current_thread));
					log::bytecode(this, op, "new", meta->name->c_str()); 
				}
				break;
			case NEWARRAY:
				{
					jtype type = (jtype)pc.get<u1>();
					jint length = stack->pop<jint>();
					log::bytecode(this, op, "new array of", type_text[type], "length", length);
					stack->push(current_thread->get_env()->create_basic_array(type, length));
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
					log::bytecode(this, op, "anewarray", ca->name->c_str());
				}
				break;
			case ARRAYLENGTH:
				{
					jreference arrayref = stack->pop<jreference>();
					stack->push(current_thread->get_env()->array_length(arrayref));
					log::bytecode(this, op, "array length of", arrayref, "is", stack->top<jint>());
				}
				break;
			case ATHROW:
				{
					log::bytecode(this, op, "athrow");
					jreference e = stack->pop<jreference>();
					handle_exception(e);
				}
				break;
			case CHECKCAST:
				{
					u2 index = pc.get<u2>();
					claxx * cls = current_const_pool->get_class(index, current_thread);
					jreference ref = stack->top<jreference>();
					if (ref == null) break;
					object * obj = memery::ref2oop(ref);
					log::bytecode(this, op, "checkcast", stack->top<jreference>(), "to", cls->name->c_str());
					if (obj && !obj->meta->check_cast_to(cls)) {
						//stack->pop<jreference>();
						throw_exception("java/lang/ClassCastException");
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
					log::bytecode(this, op, "check", ref, "instance of", cls->name->c_str());
					jint result = memery::ref2oop(ref)->is_instance(cls);
					stack->push(result);
				}
				break;
			case MONITERENTER:
				{
					jreference obj = stack->pop<jreference>();
					log::bytecode(this, op, "monitor enter", obj);
					object::from_reference(obj)->mnt.enter(current_thread);
				}
				break;
			case MONITEREXIT:
				{
					jreference obj = stack->pop<jreference>();
					log::bytecode(this, op, "monitor exit", obj);
					object::from_reference(obj)->mnt.exit(current_thread);
				}
				break;
			case IFNULL:
				{
					int16_t offset = pc.get<int16_t>();
					jreference obj = stack->pop<jreference>();
					log::bytecode(this, op, "ifnul", obj, "goto", current_pc + offset);
					if (!obj) pc.pos(current_pc+offset);
				}
				break;
			case IFNONNULL:
				{
					jreference ref = stack->pop<jreference>();
					jshort offset = pc.get<u2>();
					log::bytecode(this, op, "ifnonnull", ref,"goto", current_pc+offset);
					if (ref) pc.pos(current_pc + offset);
				}
				break;
			default:
				printf("unkown instruction 0x%02x\n", op);
				fflush(stdout);
				abort();
		}
		}catch (const char * name) {
			throw_exception(name);
		}catch (jreference e) {
			handle_exception(e);
		}
		//print_stack();
		//print_locals();
		//fflush(stdout);
	}
#if 0
	if (current_method->ret_type != T_VOID) {
		if (array::slot_need2(current_method->ret_type) == array::slot_need<jlong>::value) {
			return stack->pop<jlong>();
		}else {
			return stack->pop<jint>();
		}
	}
	return 0;
#endif
	return 0;
}

