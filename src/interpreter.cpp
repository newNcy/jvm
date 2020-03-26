#include "interpreter.h"
#include "thread.h"
#include <iterator>
#include <string>

void debug(const char * f, ...)
{
	va_list l;
	va_start(l,f);
	char buff[2048] = {0};
	vsprintf(buff, f, l);
	printf("%s", buff);
	va_end(l);
}

void frame::exec()
{
	while (pc.value()) {
		debug("[%05d] ", pc.rd);
		u1 op = pc.get<u1>();
		debug("0x%02x ", op);
		switch (op) {
			case 0:
				debug("nop\n");
				break;
			case ACONST_NULL:
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
					stack->push<jfloat>(op - FCONST_0);
				}
				break;
			case BIPUSH:
				stack->push<jint>(pc.get<u1>());
				break;
			case LDC:
				{
					u1 index = pc.get<u1>();
					const_pool_item * c = current_const_pool->get(index);
					if (c->tag == CONSTANT_Long || c->tag == CONSTANT_Double) {
						jlong v = c->value.as_long;
						debug("ldc %d[%ld]\n", index, v);
						stack->push<jlong>(v);
					}else {
						jreference v = c->value.as_reference;
						debug("ldc %d[%d]\n", index, v);
						stack->push<jreference>(v);
					}
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
					if (a == b) pc_offset(pc.get<u2>());
				}
				break;
			case IF_ACMPNE:
				{
					jreference a = stack->pop<jreference>();
					jreference b = stack->pop<jreference>();
					if (a != b) pc_offset(pc.get<u2>());
				}
				break;
			case GETSTATIC: 
				{
					field * f = current_const_pool->get_field(pc.get<u2>(), current_thread);
					member_operator members(f->owner->static_members, f->owner->static_size());
					if (f->type > OBJECT) {
						stack->push(members.get<jlong>(f->offset));
					}else {
						stack->push(members.get<jint>(f->offset));
					}
				}
				break;
			case PUTSTATIC:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					member_operator members(f->owner->static_members, f->owner->static_size());
					if (f->type > OBJECT) {
						members.put(stack->pop<jlong>(), f->offset);
					}else {
						members.put(stack->pop<jint>(), f->offset);
					}
				}
				break;
			case GETFIELD:
				{
					u2 index = pc.get<u2>();
					field * f = current_const_pool->get_field(index, current_thread);
					jreference objref = stack->pop<jreference>();
					object * obj = memery::ref2oop(objref);
					member_operator members(obj->data, obj->meta->size());
					if (f->type > OBJECT) {
						stack->push(members.get<jlong>(f->offset));
					}else {
						stack->push(members.get<jint>(f->offset));
					}
				}
				break;
			case PUTFIELD:
				{

					u2 index = pc.get<u2>();
					debug("putfield %d\n",index);
					field * f = current_const_pool->get_field(index, current_thread);
					if (f->type > OBJECT) {
						jlong v = stack->pop<jlong>();
						jreference objref = stack->pop<jreference>();
						object * obj = memery::ref2oop(objref);
						member_operator members(obj->data, obj->meta->size());
						members.put(v, f->offset);
					}else {
						jint v = stack->pop<jint>();
						jreference objref = stack->pop<jreference>();
						object * obj = memery::ref2oop(objref);
						member_operator members(obj->data, obj->meta->size());
						members.put(v, f->offset);
					}
				}
				break;
			case INVOKEVIRTUAL:
			case INVOKESPECIAL:
			case INVOKESTATIC:
				{
					const char * msg []  = {"virtual", "special", "static"};
					debug("invoke%s\n", msg[op-INVOKEVIRTUAL]);
					method * to_call = current_const_pool->get_method(pc.get<u2>(), current_thread);
					if (to_call) current_thread->push_frame(to_call);
				}
				break;
			case NEW:
				{
					claxx * meta = current_const_pool->get_class(pc.get<u2>(), current_thread);
					stack->push(memery::alloc_heap_object(meta));
				}
				break;
			case NEWARRAY:
				{
					u1 type = pc.get<u1>();
					if (4 <= type && type <= 11) {
					}
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
				}
				break;
			case MONITEREXIT:
				{
					debug("monitor exit\n");
				}
				break;
			case IFNULL:
				if (stack->pop<jreference>()) pc_offset(pc.get<u2>());
				break;
			case IFNONNULL:
				{
					jreference ref = stack->pop<jreference>();
					debug("ifnonnull %d\n", ref);
					if (ref) pc_offset(pc.get<u2>());
				}
				break;
			default:
				debug("unkown instruction 0x%02x\n", op);
				fflush(stdout);
				abort();
		}
		any_array * l = locals;
		for (int i = 0 ; i < l->N; i ++) {
			printf("%d ",l->data[i]);
		}
		printf("\n");
	}
}

