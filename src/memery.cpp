#include "memery.h"
#include "class.h"
#include "classloader.h"
#include "object.h"
#include "log.h"

std::map<jreference, object*> memery::ref_oop_map;
jreference memery::alloc_heap_object(claxx * meta, bool is_static)
{
	if (!meta) return null;
	jreference ref = ref_oop_map.size() + 1;
	uint32_t size = sizeof(object) + (is_static ? meta->static_member_size : meta->size());
	object * oop = (object*)new char[size]();
	new (oop) object(size);
	oop->meta = meta;
	ref_oop_map[ref] = oop;
	if (is_static) meta->static_obj = ref;
	log::debug("[%d] %s [%d] bytes", ref, meta->name->c_str(), size);
	return ref;
}

jreference memery::alloc_heap_array(claxx * meta, size_t length)
{
	if (!meta || !meta->is_array()) return null;
	jreference ref = ref_oop_map.size() + 1;
	uint32_t size = sizeof(object) + meta->size(length);
	object * oop = (object*)new char[size]();
	new (oop) object(size, length);
	oop->meta = meta;
	log::debug("[%d] %s [%d] length[%d] bytes", ref, meta->name->c_str(), oop->array_length(), size);
	ref_oop_map[ref] = oop;
	return ref;
}

object * memery::ref2oop(jreference ref)
{
	auto res = ref_oop_map.find(ref);
	if ( res == ref_oop_map.end()) return nullptr;
	return res->second;
}

