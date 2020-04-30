
#include "native.h"
#include "log.h"


NATIVE jreference sun_reflect_NativeMethodAccessorImpl_invoke0(environment * env, jreference cls,  jreference m, jreference obj, jreference args) 
{
	auto clazz_id = env->lookup_field_by_object(m, "clazz");
	auto slot_id = env->lookup_field_by_object(m, "slot");
	jreference clazz = env->get_object_field(m, clazz_id);
	jint slot = env->get_object_field(m, slot_id);

	claxx * owner = claxx::from_mirror(clazz, env->get_thread());
	method * mp = owner->method_by_index[slot];
	log::trace("invoke0 %s.%s %d", owner->name->c_str(), mp->name->c_str(), mp->is_static());
	jint arg_size = env->array_length(args);
	if (!mp->is_static()) {
		arg_size ++;
	}
	array_stack arg_pack(arg_size);
	if (!mp->is_static()) { 
		arg_pack.push(obj);
	}

	for (int i = 0 ; i <  arg_size; i ++) {
		arg_pack.push(env->get_array_element(args, i).l);
	}

	return env->callmethod(mp, arg_pack);
}
