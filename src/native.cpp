#include "native.h"
#include "class.h"
#include "thread.h"
#include <cstdio>









NATIVE void java_lang_Object_registerNatives(environment * env)
{
	printf("java_lang_Object_registerNatives called\n");
}

NATIVE void java_lang_System_registerNatives(environment * env)
{
	printf("java_lang_System_registerNatives called\n");
}

NATIVE void java_lang_System_loadLibrary(environment * env)
{
	printf("java_lang_System_loadLibrarycalled\n");
}
NATIVE void java_lang_System_initProperties(environment * env)
{
	printf("java_lang_System_initProperties called\n");
}

NATIVE int Test_test(environment * env, int arg) 
{
	printf("Tes.test called arg:%d\n", arg);
	return arg * 2;
}
