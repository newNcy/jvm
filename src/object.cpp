#include "object.h"
#include "memery.h"

object * object::from_reference(jreference ref)
{
	return memery::ref2oop(ref);
}
