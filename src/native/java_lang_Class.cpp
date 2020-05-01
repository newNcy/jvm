
#include "native.h"
#include "jvm.h"
#include "log.h"
#include "classloader.h"
#include <cstdio>
#include <cstdlib>

NATIVE int java_lang_Class_desiredAssertionStatus0(environment * env, jreference ref) 
{
	return 1;
}
NATIVE void java_lang_Class_registerNatives(environment * env,jreference cls)
{
	log::debug("java_lang_Object_registerNatives called %d\n", cls);
}

NATIVE jreference java_lang_Class_getPrimitiveClass(environment * env, jreference cls,  jreference ref)
{
	log::debug("java_lang_Class_getPrimitiveClass called %d prop ref %d\n", cls, ref);
	const std::string & bytes = env->get_utf8_string(ref);
	log::debug("%s --> %d\n", bytes.c_str(), env->get_vm()->primitive_types[bytes]);
	return env->get_vm()->primitive_types[bytes];
}

NATIVE jboolean java_lang_Class_isPrimitive(environment * env, jreference obj)
{
	claxx * meta = env->get_vm()->get_class_loader()->claxx_from_mirror(obj);
	log::debug("%d --> %s\n", obj, meta->name->c_str());
	return meta->is_primitive();
}

NATIVE jboolean java_lang_Class_isAssignableFrom(environment * env, jreference sup, jreference sub)
{
	log::debug("%d <- %d\n", sup, sub);
	if (sup == sub) return true;
	auto a = env->get_vm()->get_class_loader()->claxx_from_mirror(sup);
	auto b = env->get_vm()->get_class_loader()->claxx_from_mirror(sub);
	return b->check_cast_to(a);
}

NATIVE jreference java_lang_Class_getDeclaredFields0(environment * env, jreference cls, jboolean z)
{
	auto ec = env->get_vm()->get_class_loader()->load_class("java/lang/reflect/Field", env->get_thread());
	auto cls_meta = env->get_vm()->get_class_loader()->claxx_from_mirror(cls);
	jreference ret = env->create_obj_array(ec->mirror, cls_meta->fields.size());
	auto name = env->lookup_field_by_class(ec->mirror, "name");
	auto type = env->lookup_field_by_class(ec->mirror, "type");
	auto clazz = env->lookup_field_by_class(ec->mirror, "clazz");
	auto modifiers = env->lookup_field_by_class(ec->mirror, "modifiers");
	int i = 0;
	for (auto f : cls_meta->fields) {
		jreference e = memery::alloc_heap_object(ec);
		jreference nv = env->create_string_intern(f.second->name->c_str());
		auto fm = f.second->get_meta(env->get_thread())->mirror;

		env->set_object_field(e, name, nv);
		env->set_object_field(e, type, fm);
		env->set_object_field(e, clazz, cls);
		env->set_object_field(e, modifiers, f.second->access_flag);
		env->set_array_element(ret, i++, e);
	}
	return ret;
}

NATIVE jreference java_lang_Class_getInterfaces0(environment * env, jreference cls)
{
	claxx * meta = claxx::from_mirror(cls, env->get_thread());
	log::trace("getinterface of %s", meta->name->c_str());
	claxx * ret_type = env->bootstrap_load("[Ljava/lang/Class;");

	jreference ret = ret_type->instantiate(meta->interfaces.size(), env->get_thread());
	int i = 0;
	for (auto it : meta->interfaces) {
		log::trace("add %s %d", it->name->c_str(), it->mirror);
		env->set_array_element( ret, i++, it->mirror);
	}
	return ret;
}


NATIVE jreference java_lang_Class_getDeclaredMethods0(environment * env, jreference cls, jboolean z)
{
	auto ec = env->bootstrap_load("java/lang/reflect/Method");
	auto cls_meta = claxx::from_mirror(cls, env->get_thread());


	jreference ret = env->create_obj_array(ec->mirror, cls_meta->method_by_index.size());

	auto name = env->lookup_field_by_class(ec->mirror, "name");
	auto clazz = env->lookup_field_by_class(ec->mirror, "clazz");
	auto modifiers = env->lookup_field_by_class(ec->mirror, "modifiers");
	auto slot = ec->lookup_field("slot");
	auto param_types = ec->lookup_field( "parameterTypes");
	
	auto ca = param_types->get_meta(env->get_thread());

	int i = 0;
	for (auto ms : cls_meta->methods) {
		for (auto m : ms.second) {
			jreference e = memery::alloc_heap_object(ec);
			jreference nv = env->create_string_intern(m.second->name->c_str());

			jreference ps = ca->instantiate(m.second->arg_types.size(), env->get_thread());
			
			if (!m.second->arg_types.empty()) {
				int idx = 0;
				for (auto p : m.second->param_types) {
					env->set_array_element(ps, idx++, env->lookup_class(p->c_str()));
				}
			}
			env->set_object_field(e, name, nv);
			env->set_object_field(e, clazz, cls);
			env->set_object_field(e, modifiers, m.second->access_flag);
			env->set_object_field(e, param_types, ps);
			env->set_object_field(e, slot, m.second->slot);
			env->set_array_element(ret, i ++, e);
		}
	}
	return ret;
}

NATIVE jreference java_lang_Class_getDeclaredConstructors0(environment * env, jreference cls, jboolean z)
{
	claxx * constructor = env->get_vm()->get_class_loader()->load_class("java/lang/reflect/Constructor", env->get_thread());
	claxx * constructor_array = constructor->get_array_claxx(env->get_thread());
	if (!constructor_array) return null;
	claxx * c = claxx::from_mirror(cls, env->get_thread());
	if (!c) return null;

	auto cons = c->constructors();
	if (cons.empty()) return null;

	auto ret = constructor_array->instantiate(cons.size(), env->get_thread());
	field * param_types = constructor->lookup_field( "parameterTypes");
	field * modifiers = constructor->lookup_field("modifiers");
	field * clazz = constructor->lookup_field("clazz");
	field * slot = constructor->lookup_field("slot");

	auto ca = param_types->get_meta(env->get_thread());

	int idx = 0;
	for (auto con : cons) {
		jreference a = constructor->instantiate( env->get_thread());
		jreference ps = ca->instantiate(con->arg_types.size(), env->get_thread());
		if (!con->arg_types.empty()) {
			int i = 0;
			for (auto p : con->param_types) {
				env->set_array_element(ps, i++, env->lookup_class(p->c_str()));
			}
		}
		env->set_object_field(a, param_types, ps);
		jvalue mdf;
		mdf.i = con->access_flag;
		env->set_object_field(a, modifiers, mdf.i);
		env->set_object_field(a, clazz, cls);
		env->set_object_field(a, slot, con->slot);
		log::debug("set %d clazz %d\n", a, cls);
		log::debug("set %d modifiers %d\n", a, mdf.i);
		env->set_array_element(ret, idx ++, a);
	}
	return ret;
}



NATIVE jreference java_lang_Class_getName0(environment * env, jreference cls)
{
	claxx * meta = env->get_vm()->get_class_loader()->claxx_from_mirror(cls);
	std::string name = meta->name->c_str();
	for (auto & c : name) {
		if (c == '/') c = '.';
	}
	return env->create_string_intern(name);
}

NATIVE jreference java_lang_Class_forName0(environment * env, jreference cls, jreference name)
{
	std::string name_utf = env->get_utf8_string(name);
	for (char & c : name_utf) {
		if (c == '.') c = '/';
	}
	auto meta = env->get_vm()->get_class_loader()->load_class(name_utf, env->get_thread());
	if (meta) return meta->mirror;
	return null;
}

NATIVE jboolean java_lang_Class_isInterface(environment * env, jreference cls)
{
	claxx * meta = env->get_vm()->get_class_loader()->claxx_from_mirror(cls);
	return meta->is_abstract();
}

NATIVE jint java_lang_Class_getModifiers(environment * env, jreference cls)
{
	claxx * meta = env->get_vm()->get_class_loader()->claxx_from_mirror(cls);
	return meta->access_flag;
}

NATIVE jreference java_lang_Class_getSuperclass(environment * env, jreference cls)
{
	claxx * meta = claxx::from_mirror(cls, env->get_thread());
	if (meta->super_class) {
		return meta->super_class->mirror;
	}
	return null;
}

NATIVE jboolean java_lang_Class_isArray(environment * env, jreference cls)
{
	return env->class_is_array(cls);
}

NATIVE jreference java_lang_Class_getComponentType(environment * env, jreference cls)
{
	return env->array_component(cls);
}

NATIVE jreference java_lang_Class_getEnclosingMethod0(environment * env, jreference cls)
{
	return env->get_enclosing_method(cls);
}


NATIVE jreference java_lang_Class_getDeclaringClass0(environment * env, jreference cls)
{
	return env->get_declaring_class(cls);
}

NATIVE jboolean java_lang_Class_isInstance(environment * env, jreference thix, jreference obj)
{
	claxx * T = claxx::from_mirror(thix, env->get_thread());
	claxx * S = claxx::from_mirror(env->get_class(obj), env->get_thread());
	return S->check_cast_to(T);
}
