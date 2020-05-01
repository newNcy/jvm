
#include "native.h"
#include <map>
#include <string>
#include "log.h"


NATIVE jreference java_lang_ClassLoader_registerNatives(environment * env, jreference cls)
{
}

NATIVE jreference java_lang_ClassLoader_findBuiltinLib(environment * env, jreference loader, jreference name)
{
	return name;
}

NATIVE void java_lang_ClassLoader_NativeLibrary_load(environment * env, jreference thix, jreference name, jboolean builtin)
{
	auto f = env->lookup_field_by_object(thix, "loaded");
	env->set_object_field(thix, f, true);
}

NATIVE jreference java_lang_ClassLoader_findLoadedClass0(environment * env, jreference thix, jreference name_ref)
{
	std::string name = env->get_utf8_string(name_ref);
	for (auto & c : name) {
		if (c == '.') c = '/';
	}
	log::debug("find %s", name.c_str());
	return env->lookup_class(name);
}

NATIVE jreference java_lang_ClassLoader_findBootstrapClass(environment * env, jreference thix, jreference name_ref)
{
	return java_lang_ClassLoader_findLoadedClass0(env, thix, name_ref);
}
