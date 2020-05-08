
#include "native.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <mutex>
#include <utility>
#include "log.h"

fieldID fosfd = 0;
NATIVE void java_io_FileOutputStream_initIDs(environment * env, jreference cls)
{
	fosfd = env->lookup_field_by_class(cls, "fd");
}

struct io_buffer
{
	int fd = -1;
	constexpr static int len = 2048;
	char * buf = nullptr;
	int use = 0;
	int write(char * data, int length)
	{
		if (length >= len) {
			return ::write(fd, data, length);
		}else {
			if (length > len - use) {
				flush();
			}
			memcpy(buf + use, data, length);
			use += length;
		}
		if (strchr(data, '\n')) flush();
		return length;
	}
	void flush()
	{
		::write(fd, buf, use);
		use = 0;
	}

	io_buffer & operator = (io_buffer && o)
	{
		io_buffer(std::forward<io_buffer>(o));
	}
	io_buffer(io_buffer && o) 
	{
		use = o.use;
		buf = o.buf;
		fd = o.fd;
		o.buf = nullptr;
	}
	io_buffer()
	{
		buf = new char[len];
		use = 0;
	}
	~io_buffer()
	{
		if (buf) delete [] buf;
	}
};

std::map<int, io_buffer> buffers;

std::mutex mtx;
NATIVE jint java_io_FileOutputStream_writeBytes(environment * env, jreference fos, jreference bs, jint start, jint length, jboolean z)
{
	fosfd = env->lookup_field_by_object(fos, "fd");
	jreference file_disc = env->get_object_field(fos, fosfd);
	fieldID fd_id = env->lookup_field_by_object(file_disc, "fd");

	jint fd = env->get_object_field(file_disc, fd_id);
	object * oop = object::from_reference(bs);
	mtx.lock();
	int ret = 0;
#if 1
	auto buf = buffers.find(fd);
	if (buf != buffers.end()) {
		ret = buf->second.write(oop->data + start, length);
	}else {
		auto & buffer = buffers[fd];
		buffer.fd = fd;
		ret = buffer.write(oop->data + start, length);
	}
#else 
	ret = write(fd, oop->data + start, length);
#endif
	mtx.unlock();
	return ret;
}


