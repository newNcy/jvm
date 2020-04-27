
#include "native.h"
fieldID fd = 0;
NATIVE void java_io_FileDescriptor_initIDs(environment * env, jreference cls) 
{
	fd = env->lookup_field_by_class(cls, "fd");
}


