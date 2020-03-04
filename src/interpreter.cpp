#include "interpreter.h"

void debug(const char * f, ...)
{
	va_list l;
	va_start(l,f);
	char buff[2048] = {0};
	vsprintf(buff, f, l);
	printf("%s", buff);
	va_end(l);
}

void thread::run()
{
	while (current_frame) {
		if (!code->value()) { 
			pop_frame();
			continue;
		}

		debug("[%05d] ", code->rd);
		u1 op = code->get<u1>();
		debug("0x%02x ", op);
		switch (op) {
			case 0:
				debug("nop\n");
				break;
			case ACONST_NULL:
				current_stack.push<jreference>(0);
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
					current_stack.push<jint>(op - ICONST_M1 - 1);
				}
				break;
			case FCONST_0:
			case FCONST_1:
			case FCONST_2:
				{
					current_stack.push<jfloat>(op - FCONST_0);
				}
				break;
			case BIPUSH:
				current_stack.push<jint>(code->get<u1>());
				break;
			case LDC:
				{
					u1 index = code->get<u1>();
					const_pool_item * c = current_const_pool->get(index);
					if (c->tag == CONSTANT_Long || c->tag == CONSTANT_Double) {
						jlong v = c->value.as_long;
						debug("ldc %d[%ld]\n", index, v);
						current_stack.push<jlong>(v);
					}else {
						jreference v = c->value.as_reference;
						debug("ldc %d[%d]\n", index, v);
						current_stack.push<jreference>(v);
					}
				}
				break;
			case ILOAD_0:
			case ILOAD_1:
			case ILOAD_2:
			case ILOAD_3:
				{
					debug("iload %d\n", op - ILOAD_0);
					current_stack.push(current_frame->locals->get<jint>(op-ILOAD_0));
				}
				break;
			case ALOAD_0:
			case ALOAD_1:
			case ALOAD_2:
			case ALOAD_3:
				debug("aload %d\n",op - ALOAD_0);
				current_stack.push<jreference>(current_frame->locals->get<jreference>(op -ALOAD_0));
				break;
			case LSTORE:
				current_frame->locals->put(current_stack.pop<jlong>(),code->get<u1>());
				debug("lstore\n");
				break;
			case ISTORE_0:
			case ISTORE_1:
			case ISTORE_2:
			case ISTORE_3:
				debug("istore %d\n",op - ISTORE_0);
				current_frame->locals->put(current_stack.pop<jint>(), code->get<u1>());
				break;
			case ASTORE_0:
			case ASTORE_1:
			case ASTORE_2:
			case ASTORE_3:
				debug("astore %d\n", op - ASTORE_0);
				current_frame->locals->put<jreference>(current_stack.pop<jreference>(), op-ASTORE_0);
				break;
			case POP:
				debug("pop\n");
				current_stack.pop<jint>();
				break;
			case POP2:
				debug("pop2\n");
				current_stack.pop<jlong>();
				break;
			case DUP:
				{
					debug("dup\n");
					jint top = current_stack.top<jint>();
					current_stack.push(top);
				}
				break;
			case IADD:
				{
					jint a = current_stack.pop<jint>();
					jint b = current_stack.pop<jint>();
					current_stack.push<jint>(a+b);
					printf("iadd %d %d\n", a, b);
				}
				break;
			case IF_ACMPEQ:
				{
					jreference a = current_stack.pop<jreference>();
					jreference b = current_stack.pop<jreference>();
					if (a == b) pc_offset(code->get<u2>());
				}
				break;
			case IF_ACMPNE:
				{
					jreference a = current_stack.pop<jreference>();
					jreference b = current_stack.pop<jreference>();
					if (a != b) pc_offset(code->get<u2>());
				}
				break;
			case GETSTATIC: 
				{
					field * f = current_const_pool->get_field(code->get<u2>(), this);
					member_operator members(f->owner->static_members, f->owner->static_size());
					if (f->type > OBJECT) {
						current_stack.push(members.get<jlong>(f->offset));
					}else {
						current_stack.push(members.get<jint>(f->offset));
					}
				}
				break;
			case PUTSTATIC:
				{
					u2 index = code->get<u2>();
					field * f = current_const_pool->get_field(index, this);
					member_operator members(f->owner->static_members, f->owner->static_size());
					if (f->type > OBJECT) {
						members.put(current_stack.pop<jlong>(), f->offset);
					}else {
						members.put(current_stack.pop<jint>(), f->offset);
					}
				}
				break;
			case GETFIELD:
				{
					u2 index = code->get<u2>();
					field * f = current_const_pool->get_field(index, this);
					jreference objref = current_stack.pop<jreference>();
					object * obj = memery::ref2oop(objref);
					member_operator members(obj->data, obj->meta->size());
					if (f->type > OBJECT) {
						current_stack.push(members.get<jlong>(f->offset));
					}else {
						current_stack.push(members.get<jint>(f->offset));
					}
				}
				break;
			case PUTFIELD:
				{

					u2 index = code->get<u2>();
					debug("putfield %d\n",index);
					field * f = current_const_pool->get_field(index, this);
					if (f->type > OBJECT) {
						jlong v = current_stack.pop<jlong>();
						jreference objref = current_stack.pop<jreference>();
						object * obj = memery::ref2oop(objref);
						member_operator members(obj->data, obj->meta->size());
						members.put(v, f->offset);
					}else {
						jint v = current_stack.pop<jint>();
						jreference objref = current_stack.pop<jreference>();
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
					method * to_call = current_const_pool->get_method(code->get<u2>(), this);
					if (to_call) push_frame(to_call);
				}
				break;
			case NEW:
				{
					claxx * meta = current_const_pool->get_class(code->get<u2>(), this);
					current_stack.push(memery::alloc_heap_object(meta));
				}
				break;
			case ATHROW:
				{
					debug("athrow\n");
					handle_exception();
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
				if (current_stack.pop<jreference>()) pc_offset(code->get<u2>());
				break;
			case IFNONNULL:
				{
					jreference ref = current_stack.pop<jreference>();
					debug("ifnonnull %d\n", ref);
					if (ref) pc_offset(code->get<u2>());
				}
				break;
			default:
				debug("unkown instruction 0x%02x\n", op);
				fflush(stdout);
				abort();
		}
		any_array * l = current_frame->locals;
		for (int i = 0 ; i < l->N; i ++) {
			printf("%d ",l->data[i]);
		}
		printf("\n");
		print_stack();
	}
}

