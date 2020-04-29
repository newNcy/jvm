#pragma once

#include "classfile.h"
#include "frame.h"
#include <alloca.h>
#include <cstddef>
#include <cstring>
#include <new>
#include <stdexcept>
#include <stdio.h>
#include <sstream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdarg.h>

#define BUFFER_MSG(B, L, R)		\
	va_list l;					\
	va_start(l,f);				\
	char B[L] = {0};			\
	R = vsprintf(B, f, l);		\
	va_end(l);



struct log
{
	static int debug(const char * f, ...);
	static int trace(const char * f, ...);
	template <typename ... Args>
	static void bytecode(frame * f, u1 op, const char *,Args ... args);
};


struct logstream
{
	size_t n = 0;
	size_t p = 0;
	char * buf = nullptr;
	logstream(size_t N):n(N)
	{
		buf = (char*)new char[n]();
	}

	int printf(const char * f, ...);
	
	void print() {}
	template <typename T>
	void print(T t);

	template <typename T, typename ... Args>
	void print (T t, Args ... args) 
	{
		print(t);
		print(' ');
		print(args...);
	}
	void show(FILE * out = stdout)
	{
		fprintf(out, "%s\n", buf);
		fflush(out);
	}
	~logstream()
	{
		delete [] buf;
	}
};

template <>
inline void logstream::print(int i)
{
	printf("%d",i);
}
template <>
inline void  logstream::print(char c)
{
	if (0x20 <= c && c <= 0x7e) {
		if (p < n - 3) {
			buf[p ++] = c;
		}
	} else {
		print('\\');
		print<int>(c);	
	}
}

template <>
inline void  logstream::print(unsigned char c)
{
	print<char>(c);
}

template <>
inline void logstream::print(const char *s)
{
	if (s) {
		int i = 0; 
		int left = n - p - 3;
		while (*(s+i) && i < left) {
			print(s[i++]);
		}
	}
}

template <>
inline void logstream::print(const std::string & s)
{
	print(s.c_str());
}

	template <>
inline void logstream::print(std::string s)
{
	print<const std::string & >(s);
}

template <>
inline void logstream::print(unsigned int i)
{
	printf("%d",i);
}


template <>
inline void logstream::print(short i)
{
	printf("%d",i);
}

template <>
inline void logstream::print(unsigned short i)
{
	printf("%d",i);
}

template <>
inline void logstream::print(long i)
{
	printf("%ldL",i);
}


template <>
inline void logstream::print(float i)
{
	printf("%f",i);
}

template <>
inline void logstream::print(double i)
{
	printf("%lf",i);
}


template <typename ... Args> 
void log::bytecode (frame * f, u1 op, const char * text, Args ... args)
{
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

	int sz = size.ws_col;
	if (!sz || sz > 10000) {
	}
	sz = 210;

	logstream ss(2*sz);
	ss.printf(" [%s.%s::%05d/%05d] %s ", f->current_class->name->c_str(), f->current_method->name->c_str(), f->current_pc, f->code->code_length, text);
	ss.print(args...);
	ss.print(' ');

	logstream right(2*sz);
	for (auto i = f->stack->top_pos() - 1; i > f->stack->begin()-1; i --) {
		right.printf(" %d",*i);
	}
	right.printf(" [%d/%ld]", f->stack->top_pos() - f->stack->begin(), f->stack->size());
	if (f->locals->size()) {
		right.print(" | ");
		for (auto i : *f->locals) {
			right.printf("%d ", i);
		}
		right.print("|");
	}

	int space = sz - right.p - 1;
	for (int i = ss.p ; i <  space; i++) {
		if (i%3 == 0) {
			ss.print('-');
		}else {
			ss.print(' ');
		}
	}

	ss.printf("%s", right.buf);
	//printf("%s\n", ss.buf);
	ss.show();
}
