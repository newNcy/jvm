
#include "native.h"

NATIVE void java_lang_System_registerNatives(environment * env,jreference cls)
{
	printf("java_lang_System_registerNatives called %d\n",cls);
}


NATIVE void java_lang_System_loadLibrary(environment * env, jreference cls, jreference name)
{
	printf("java_lang_System_loadLibrarycalled %d %d\n", cls, name);
}

NATIVE void java_lang_System_initProperties(environment * env, jreference cls,  jreference prop)
{
	printf("java_lang_System_initProperties called %d prop ref %d\n", cls, prop);
}


