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

	LCONST_0		,
	LCONST_1		,
	FCONST_0		= 0x0b,
	FCONST_1		,
	FCONST_2		,

	DCONST_0		= 0x0e,
	DCONST_1		,

	BIPUSH			= 0x10,
	SIPUSH			= 0x11,
	LDC				= 0x12,
	LDC_W			= 0x13,
	LDC2_W			= 0x14,
	ILOAD			= 0x15,
	LLOAD			= 0x16,
	ILOAD_0			= 0x1a,
	ILOAD_1			,
	ILOAD_2			,
	ILOAD_3			,

	ALOAD			= 0x19,
	LLOAD_0			= 0x1e,
	LLOAD_1			= 0x1f,
	LLOAD_2			= 0x20,
	LLOAD_3			= 0x21,
	FLOAD_0			= 0x22,
	FLOAD_1			,
	FLOAD_2			,
	FLOAD_3			,
	ALOAD_0			= 0x2a,
	ALOAD_1			= 0x2b,
	ALOAD_2			= 0x2c,
	ALOAD_3			= 0x2d,
	IALOAD			= 0x2e,

	AALOAD			= 0x32,
	CALOAD			= 0x34,
	ISTORE			= 0x36,
	LSTORE			= 0x37,
	ASTORE			= 0x3a,

	ISTORE_0		= 0x3b,
	ISTORE_1		= 0x3c,
	ISTORE_2		= 0x3d,
	ISTORE_3		= 0x3e,
	LSTORE_0		= 0x3f,

	FSTORE_0		= 0x43,
	FSTORE_1		,
	FSTORE_2		,
	FSTORE_3		,
	ASTORE_0		= 0x4b,
	ASTORE_1		= 0x4c,
	ASTORE_2		= 0x4d,
	ASTORE_3		= 0x4e,
	IASTORE			= 0x4f,
	AASTORE			= 0x53,
	BASTORE			= 0x54,
	CASTORE			= 0x55,
	POP				= 0x57,
	POP2			,
	DUP				= 0x59,
	DUP_X1			= 0x5a,
	DUP2			= 0x5c,
	IADD			= 0x60,
	LADD			= 0x61,
	ISUB			= 0x64,
	IMUL			= 0x68,
	FMUL			= 0x6a,
	IDIV			= 0x6c,
	FDIV			= 0x6e,
	IREM			= 0x70,
	INEG			= 0x74,
	ISHL			= 0x78,
	LSHL			= 0x79,
	ISHR			= 0x7a,
	IOR				= 0x80,
	IUSHR			= 0x7c,
	IAND			= 0x7e,
	LAND			= 0x7f,
	IXOR			= 0x82,
	IINC			= 0x84,
	I2L				= 0x85,
	I2F				= 0x86,
	F2I				= 0x8b,
	I2C				= 0x92,
	FCMPL			= 0x95,
	FCMPG			= 0x96,
	IFEQ			= 0x99,
	IFNE			= 0x9a,
	IFLT			,
	IFGE			= 0x9c,
	IFGT			,
	IFLE			,
	IF_ICMPEQ		,
	IF_ICMPNE		= 0xa0,
	IF_ICMPLT		= 0xa1,
	IF_ICMPGE		= 0xa2,
	IF_ICMPGT		= 0xa3,
	IF_ICMPLE		= 0xa4,
	IF_ACMPEQ		= 0xa5,
	IF_ACMPNE		,
	GOTO			,
	LOOKUPSWITCH	= 0xab,
	IRETURN			= 0xac,
	LRETURN			= 0xad,
	FRETURN			= 0xae,
	DRETURN			= 0xaf,
	ARETURN			= 0xb0,
	RETURN			= 0xb1,
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
	CHECKCAST		= 0xc0,
	INSTANCEOF		= 0xC1,
	MONITERENTER	= 0xc2,
	MONITEREXIT		= 0xc3,
	IFNULL			= 0xc6,
	IFNONNULL		= 0xc7,
};

