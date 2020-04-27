#include "log.h"
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
	ret = fprintf(stdout, "%s\n", buff);
	fflush(stdout);
	return ret;
}
	
int logstream::printf(const char * f, ...)
{
	int ret = 0;
	BUFFER_MSG(tmp, 1024, ret);
	if (ret < n-p) {
		print<const char*>(tmp);
	}
	return ret;
}

