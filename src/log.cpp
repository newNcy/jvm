#include "log.h"
#include "memery.h"
#include "object.h"
#include <cstdio>
#include <stdarg.h>


int log::debug(const char * f, ...)
{
	int ret = 0;
	BUFFER_MSG(buff, 4096, ret);
	ret = fprintf(stderr, "%s\n", buff);
	fflush(stderr);
	return ret;
}

int log::trace(const char * f, ...)
{
	int ret = 0;
	BUFFER_MSG(buff, 4096, ret);
	ret = fprintf(stderr, "%s\n", buff);
	fflush(stderr);
	return ret;
}
	
int logstream::printf(const char * f, ...)
{

	int ret = 0;
	BUFFER_MSG(tmp, 1024, ret);
	if (ret < n-p - 1) {
		print<const char*>(tmp);
	}
	return ret;
}

	
void log::object(jreference obj)
{
	return;
	::object * oop = object::from_reference(obj);
	claxx * meta = oop->meta;
	log::trace("[%d] of class %s", obj, meta->name->c_str());
}
