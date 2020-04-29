
#include "native.h"
#include "thread.h"
#include "log.h"

NATIVE jreference java_security_AccessController_doPrivileged(environment * env, jreference cls, jreference obj)
{
	log::debug("java_security_AccessController_doPrivileged %d %d\n", cls,obj);
	methodID m = env->lookup_method_by_object(obj, "run", "()Ljava/lang/Object;"); 
	return env->callmethod(m, obj);
}

NATIVE jreference java_security_AccessController_getStackAccessControlContext(environment * env, jreference cls, jreference obj)
{
	//TODO ...
	return 0;
}

