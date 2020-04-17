
#include "native.h"
#include "thread.h"

NATIVE jreference java_security_AccessController_doPrivileged(environment * env, jreference cls, jreference obj)
{
	printf("java_security_AccessController_doPrivileged %d %d\n", cls,obj);
	jreference objc = env->get_class(obj);
	methodID m = env->lookup_method(objc, "run", "()Ljava/lang/Object;"); 
	return env->callmethod(m, obj);
}

NATIVE jreference java_security_AccessController_getStackAccessControlContext(environment * env, jreference cls, jreference obj)
{
	//TODO ...
	return 0;
}

