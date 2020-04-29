
#include "native.h"
#include <cstdio>

NATIVE void java_lang_System_registerNatives(environment * env,jreference cls)
{
	printf("java_lang_System_registerNatives called %d\n",cls);
}


NATIVE void java_lang_System_loadLibrary(environment * env, jreference cls, jreference name)
{
	printf("java_lang_System_loadLibrarycalled %d %d\n", cls, name);
}

NATIVE jreference java_lang_System_initProperties(environment * env, jreference cls,  jreference prop)
{
	printf("java_lang_System_initProperties called %d prop ref %d\n", cls, prop);
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
	return prop;
}

NATIVE void java_lang_System_setIn0(environment * env, jreference cls,  jreference in)
{
	fieldID f = env->lookup_field_by_class(cls, "in");
	env->set_object_field(cls, f, in);
}

NATIVE void java_lang_System_setOut0(environment * env, jreference cls,  jreference out)
{
	fieldID f = env->lookup_field_by_class(cls, "out");
	env->set_object_field(cls, f, out);
}

NATIVE void java_lang_System_setErr0(environment * env, jreference cls,  jreference err)
{
	fieldID f = env->lookup_field_by_class(cls, "err");
	env->set_object_field(cls, f, err);
}

NATIVE void java_lang_System_arraycopy(environment * env, jreference cls, jreference a, jint as,  jreference b, jint bs, jint len)
{
	for (int i = 0 ; i < len; i ++) {
		jvalue e =  env->get_array_element(a, i + as);
		env->set_array_element(b, i+bs, e);
	} }
