
#include "native.h"

NATIVE jreference sun_misc_URLClassPath_getLookupCacheURLs(environment * env, jreference ref) 
{
	return env->bootstrap_load("[java/lang/Object;")->instantiate(0, env->get_thread());
}
