
#include "native.h"
#include "log.h"

NATIVE jint java_lang_Float_floatToRawIntBits(environment * env, jreference cls,  jfloat f)
{
	log::debug("java_lang_Float_floatToRawIntBits called %d prop ref %f\n", cls, f);
	jvalue v = f;
	return v.i;
}


