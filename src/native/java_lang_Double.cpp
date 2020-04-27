
#include "native.h"
NATIVE jlong java_lang_Double_doubleToRawLongBits(environment * env, jreference cls,  jdouble f)
{
	printf("java_lang_Double_doubleToRawLongBits called %d prop ref %lf\n", cls, f);
	jvalue v = f;
	return v.j;
}

NATIVE jdouble java_lang_Double_longBitsToDouble(environment * env, jreference cls,  jlong f)
{
	jvalue v = f;
	return v.d;
}
