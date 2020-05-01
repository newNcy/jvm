
#include "native.h"
#include <cstdio>
#include "log.h"

NATIVE void java_lang_System_registerNatives(environment * env,jreference cls)
{
	log::debug("java_lang_System_registerNatives called %d\n",cls);
}


NATIVE void java_lang_System_loadLibrar(environment * env, jreference cls, jreference name)
{
	log::debug("java_lang_System_loadLibrarycalled %d %d\n", cls, name);
}

NATIVE jreference java_lang_System_initProperties(environment * env, jreference cls,  jreference prop)
{
	log::debug("java_lang_System_initProperties called %d prop ref %d\n", cls, prop);
	methodID put = env->lookup_method_by_object(prop, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	auto put_prop = [env, put, prop] (const std::string & key, const std::string & value) {
		env->callmethod(put, prop, env->create_string_intern(key), env->create_string_intern(value));
	};
	/*
	 */

    /* file system properties */
	put_prop("line.separator", "\n");
	put_prop("file.separator", "/");
	put_prop("path.separator", "/");

	put_prop("java.vm.name", "jvm");
	put_prop("file.encoding", "UTF-8");
	put_prop("sun.jnu.encoding", "UTF-8");
	put_prop("file.encoding.pkg", "sun.io");
	put_prop("user.country", "CN");
	put_prop("user.language", "zh");
	put_prop("java.specification.version", "1.8");
	put_prop("sun.io.unicode.encoding", "UnicodeBig");
	put_prop("java.locale.providers", "JRE");
	put_prop("sun.nio.cs.bugLevel", "1.4");
	put_prop("gopherProxySet", "false");
	put_prop("java.home", "/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.242.b08-0.el7_7.x86_64/jre");
	return prop;
}

NATIVE void java_lang_System_setIn0(environment * env, jreference cls,  jreference in)
{
	fieldID f = env->lookup_field_by_class(cls, "in");
	claxx * sys = claxx::from_mirror(cls, env->get_thread());
	env->set_object_field(sys->static_obj, f, in);
}

NATIVE void java_lang_System_setOut0(environment * env, jreference cls,  jreference out)
{
	fieldID f = env->lookup_field_by_class(cls, "out");
	claxx * sys = claxx::from_mirror(cls, env->get_thread());
	env->set_object_field(sys->static_obj, f, out);
}

NATIVE void java_lang_System_setErr0(environment * env, jreference cls,  jreference err)
{
	fieldID f = env->lookup_field_by_class(cls, "err");
	claxx * sys = claxx::from_mirror(cls, env->get_thread());
	env->set_object_field(sys->static_obj, f, err);
}

NATIVE jreference java_lang_System_mapLibraryName(environment * env, jreference sys,  jreference name)
{
	log::trace("map library name %s", env->get_utf8_string(name).c_str());
	return name;
}

NATIVE void java_lang_System_arraycopy(environment * env, jreference cls, jreference a, jint as,  jreference b, jint bs, jint len)
{
	for (int i = 0 ; i < len; i ++) {
		jvalue e =  env->get_array_element(a, i + as);
		env->set_array_element(b, i+bs, e);
	} 
}

NATIVE jlong java_lang_System_nanoTime(environment * env, jreference cls) 
{
	return 0;
}
