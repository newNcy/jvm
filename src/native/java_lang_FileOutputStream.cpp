
#include "native.h"
fieldID fosfd = 0;
NATIVE void java_io_FileOutputStream_initIDs(environment * env, jreference cls)
{
	fosfd = env->lookup_field(cls, "fd");
}


