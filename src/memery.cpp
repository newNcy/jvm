#include "memery.h"
#include "classloader.h"

std::map<jreference, object*> memery::ref_oop_map;
jreference memery::alloc_heap_object(claxx * meta)
{
	if (!meta) return 0;
	jreference ref = ref_oop_map.size() + 1;
	object * oop = (object*)new char[sizeof(object) + meta->size()]();
	new (oop) object;
	oop->meta = meta;
	ref_oop_map[ref] = oop;
	printf("create instanse of %s ref:%d\n", meta->name->c_str(), ref);
	return ref;
}

jreference memery::alloc_heap_array(array_claxx * meta, size_t length)
{
}

object * memery::ref2oop(jreference ref)
{
	auto res = ref_oop_map.find(ref);
	if ( res == ref_oop_map.end()) return nullptr;
	return res->second;
}
char * memery::alloc_static_members(claxx * meta)
{
	if (!meta || !meta->static_size()) return new char[1];
	return new char[meta->static_size()]();
}
