

#include "native.h"
#include <map>
#include <string>

NATIVE jint sun_misc_Signal_findSignal(environment * env, jreference cls, jreference name_obj)
{

	std::map<std::string, jint> sigs = {
		{"HUP", 1}
	};
	std::string name = env->get_utf8_string(name_obj);
	auto sig = sigs.find(name);
	if (sig != sigs.end()) return sig->second; 
	return 0;
}

NATIVE jlong sun_misc_Signal_handle0(environment * env, jreference cls, jint i, jlong j)
{
	return 0;
}
