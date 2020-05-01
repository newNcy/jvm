
#include "native.h"
#include "log.h"

NATIVE void java_io_UnixFileSystem_initIDs (environment * env,jreference cls)
{
}

NATIVE jint java_io_UnixFileSystem_getBooleanAttributes0(environment * env,jreference thix, jreference file)
{
	auto f = env->lookup_method_by_object(file, "getName", "()Ljava/lang/String;");
	jreference name_ref = env->callmethod(f, file);
	std::string name = env->get_utf8_string(name_ref);
	log::debug("file %s", name.c_str());
	return 0;
}
