
#include "native.h"
fieldID fisfd = 0;
NATIVE void java_io_FileInputStream_initIDs(environment * env, jreference cls)
{
	fisfd = env->lookup_field_by_class(cls, "fd");
}


