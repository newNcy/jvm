
#include "native.h"
#include <unistd.h>
#include "log.h"

fieldID fosfd = 0;
NATIVE void java_io_FileOutputStream_initIDs(environment * env, jreference cls)
{
	fosfd = env->lookup_field_by_class(cls, "fd");
}

NATIVE void java_io_FileOutputStream_writeBytes(environment * env, jreference fos, jreference bs, jint start, jint length, jboolean z)
{
	fosfd = env->lookup_field_by_object(fos, "fd");
	jreference file_disc = env->get_object_field(fos, fosfd);
	fieldID fd_id = env->lookup_field_by_object(file_disc, "fd");

	jint fd = env->get_object_field(file_disc, fd_id);
	object * oop = object::from_reference(bs);
	write(fd, oop->data + start, length); 
}


