
#include "native.h"

#include <fcntl.h>
#include <unistd.h>
fieldID fisfd = 0;
extern fieldID java_io_FileDescriptor_fd;
NATIVE void java_io_FileInputStream_initIDs(environment * env, jreference cls)
{
	fisfd = env->lookup_field_by_class(cls, "fd");
}

NATIVE void java_io_FileInputStream_open0(environment * env, jreference thix, jreference name_ref)
{
	std::string path = env->get_utf8_string(name_ref);
	log::debug("open file %s", path.c_str());

	int fd = open(path.c_str(), O_RDONLY);
	jreference fdis = env->get_object_field(thix, fisfd);
	env->set_object_field(fdis, java_io_FileDescriptor_fd, fd);
	if (fd < 0) {
		env->throw_exception("java/io/FileNotFoundException", path);
	}else {
		log::trace("fd:%d", fd);
		env->set_object_field(fdis, java_io_FileDescriptor_fd, fd);
	}
}

NATIVE jint java_io_FileInputStream_readBytes(environment * env, jreference thix, jreference dst, jint start, jint length)
{
	jreference fdis = env->get_object_field(thix, fisfd);
	jint fd = env->get_object_field(fdis, java_io_FileDescriptor_fd);
	object * oop = object::from_reference(dst);
	char * buff = oop->data + start;
	return read(fd, buff, length);
}
  
NATIVE void java_io_FileInputStream_close0(environment * env, jreference thix)
{
	jreference fdis = env->get_object_field(thix, fisfd);
	jint fd = env->get_object_field(fdis, java_io_FileDescriptor_fd);
	close(fd);
}

