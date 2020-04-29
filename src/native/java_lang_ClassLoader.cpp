
#include "native.h"
#include <map>
#include <string>


NATIVE jreference java_lang_ClassLoader_registerNatives(environment * env, jreference cls)
{
}

NATIVE jreference java_lang_ClassLoader_findBuiltinLib(environment * env, jreference loader, jreference name)
{
	return name;
}

NATIVE void java_lang_ClassLoader_NativeLibrary_load(environment * env, jreference loader_n, jreference name)
{
}
