
#include "native.h"
#include "log.h"

NATIVE jreference sun_reflect_NativeConstructorAccessorImpl_newInstance0(environment * env, jreference thix, jreference con, jreference args)
{
	log::trace("cons %d , args %d", con, args);
	fieldID clazz_id = env->lookup_field_by_object(con, "clazz");
	fieldID slot_id = env->lookup_field_by_object(con, "slot");

	jreference	clazz = env->get_object_field(con, clazz_id);
	jint		slot  = env->get_object_field(con, slot_id);

	claxx * meta = claxx::from_mirror(clazz, env->get_thread());
	method * ctr = meta->method_at(slot);
	log::trace("reflect instance %s", meta->name->c_str());

	jreference ret = meta->instantiate(env->get_thread());
	log::trace("reflect instance %d", ret);

	array_stack arg_pack(ctr->arg_space + 1);
	arg_pack.push(ret);
	if (args) {
		jint count = env->array_length(args);
		while (count --) {
			//将args数组中的参数传给构造函数的过程中可能还包含有把包装类引用变成primitive type值的过程,目前唯一一次走这里的是没有参数，先不处理
		}
	}

	env->callmethod(ctr, arg_pack);
	return ret;
}
