#pragma once
#include "jvm.h"
#include <stdarg.h>
#include "util.h"

static const char * op_name = 
{
	
};

enum {
	ACONST_NULL		= 0x01,
	ICONST_M1		= 0x02,
	ICONST_0		= 0x03,
	ICONST_1		= 0x04,
	ICONST_2		= 0x05,
	ICONST_3		= 0x06,
	ICONST_4		= 0x07,
	ICONST_5		= 0x08,

	FCONST_0		= 0x0b,
	FCONST_1		,
	FCONST_2		,

	BIPUSH			= 0x10,
	LDC				= 0x12,
	
	DCONST_0		= 0x14,
	DCONST_1		,

	ILOAD_0			= 0x1a,
	ILOAD_1			,
	ILOAD_2			,
	ILOAD_3			,

	ALOAD			= 0x19,
	FLOAD_0			= 0x22,
	FLOAD_1			,
	FLOAD_2			,
	FLOAD_3			,
	ALOAD_0			= 0x2a,
	ALOAD_1			= 0x2b,
	ALOAD_2			= 0x2c,
	ALOAD_3			= 0x2d,

	ISTORE			,
	LSTORE			= 0x37,
	ASTORE			= 0x3a,

	ISTORE_0		= 0x3b,
	ISTORE_1		= 0x3c,
	ISTORE_2		= 0x3d,
	ISTORE_3		= 0x3e,
	ISTORE_4		= 0x3f,

	ASTORE_0		= 0x4b,
	ASTORE_1		= 0x4c,
	ASTORE_2		= 0x4d,
	ASTORE_3		= 0x4e,
	AASTORE			= 0x53,
	CASTORE			= 0x55,
	POP				= 0x57,
	POP2			,
	DUP				= 0x59,
	IADD			= 0x60,
	IUSHR			= 0x7c,
	IXOR			= 0x82,
	FCMPG			= 0x96,
	IFGE			= 0x9c,
	IFGT			,
	IFLE			,
	IF_ACMPEQ		= 0xa5,
	IF_ACMPNE		,
	GOTO			,
	ARETURN			= 0xb0,
	GETSTATIC		= 0xb2,
	PUTSTATIC		= 0xb3,
	GETFIELD		= 0xb4,
	PUTFIELD		= 0xb5,
	INVOKEVIRTUAL	= 0xb6,
	INVOKESPECIAL	= 0xb7,
	INVOKESTATIC	= 0xb8,
	INVOKEINTERFACE = 0xb9,
	NEW				= 0xbb,
	NEWARRAY		,
	ANEWARRAY		,
	ARRAYLENGTH		,
	ATHROW			= 0xbf,
	MONITERENTER	= 0xc2,
	MONITEREXIT		= 0xc3,
	IFNULL			= 0xc6,
	IFNONNULL		= 0xc7,
};

