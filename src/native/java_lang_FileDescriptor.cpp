
#include "native.h"

fieldID java_io_FileDescriptor_fd = 0;

NATIVE void java_io_FileDescriptor_initIDs(environment * env, jreference cls) 
{
	java_io_FileDescriptor_fd = env->lookup_field_by_class(cls, "fd");
}


