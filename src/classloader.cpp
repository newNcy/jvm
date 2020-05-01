#include "classloader.h"
#include "attribute.h"
#include "class.h"
#include "classfile.h"
#include "memery.h"
#include <cstring>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <zip.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "jvm.h"
#include "object.h"
#include "log.h"
#include "thread.h"

class raw_const_pool 
{
	std::vector<cp_info*> & raw_pool;
	const_pool * rt_pool;
	thread * current_thread = nullptr;
	
	public:
	template <typename T>
	T * get(int idx) const
	{
		if (0 < idx && idx <= raw_pool.size() && raw_pool[idx]->tag == T::TAG) {
			return (T*)raw_pool[idx];
		}else {
			abort();
		}
	}

	raw_const_pool(std::vector<cp_info*> & raw, const_pool * rt, thread * cur_thread) : raw_pool(raw), rt_pool(rt), current_thread(cur_thread) {}

	const_pool_item * parse_entry(int index) 
	{
		if (!rt_pool) return nullptr;
		if (!index) return nullptr;
		if (rt_pool->get(index)) return rt_pool->get(index);
		cp_info * info = raw_pool[index];
		if (!info) return nullptr;
		//printf("parsing entry %d tag:%d \n", index, info->tag);

		const_pool_item * ret = memery::alloc_meta<const_pool_item>();
		ret->tag = info->tag;
		switch(info->tag) {
			case CONSTANT_Class: 
				{
					symbol * name = get<symbol>(((cp_class*)info)->name_index);
					class_ref * the_class = memery::alloc_meta<class_ref>();
					the_class->name = name;
					ret->sym_class = the_class;
				}
				break;
			case CONSTANT_FieldRef:
			case CONSTANT_MethodRef:
			case CONSTANT_InterfaceMethodRef:
				{
					cp_field_ref * field = (cp_field_ref*)info;
					field_ref * ref = memery::alloc_meta<field_ref>();
					ref->class_index = field->class_index;
					cp_name_and_type * name_and_type = get<cp_name_and_type>(field->name_and_type_index);
					ref->name = get<symbol>(name_and_type->name_index);
					ref->discriptor = get<symbol>(name_and_type->discriptor_index);
					ret->sym_field = ref;
				}
				break;
			case CONSTANT_String:
				{
					//暂时不创建string
					ret->utf8_str = get<symbol>(((cp_string*)info)->string_index);
				}
				break;
			case CONSTANT_Integer:
				{
					cp_int * num = (cp_int*)info;
					ret->value.i = num->bytes;
				}
				break;
			case CONSTANT_Float:
				{
					cp_float * num = (cp_float*)info;
					ret->value.f = *(float*)&(num->bytes);
				}
				break;
			case CONSTANT_Long:
				{
					cp_long * num = (cp_long*)info;
					ret->value.j = num->bytes;
				}
				break;
			case CONSTANT_Double:
				{
					cp_double * num = (cp_double*)info;
					ret->value.l = num->bytes;
				}
				break;
			case CONSTANT_NameAndType:
				{
					cp_name_and_type * nnt = (cp_name_and_type *)info;
					ret->value.i = nnt->name_index << 16 | nnt->discriptor_index; //from hostpot
				}
				break;
			case CONSTANT_Utf8:
				ret->utf8_str = (cp_utf8*)info;
				break;
			case CONSTANT_MethodHandle:
				{
					cp_method_handle * method_handle = (cp_method_handle*)info;
					method_handle_ref * sym_method_handle = memery::alloc_meta<method_handle_ref>();
					sym_method_handle->reference_kind = method_handle->reference_kind;
					sym_method_handle->sym_field = parse_entry(method_handle->reference_index)->sym_field;
					ret->sym_method_handle = sym_method_handle;
				}
				break;
			case CONSTANT_MethodType:
				{
					cp_method_type * method_type = (cp_method_type*)info;
					method_type_ref * sym_method_type = memery::alloc_meta<method_type_ref>();
					sym_method_type->discriptor = get<symbol>(method_type->discriptor_index);
					ret->sym_method_type = sym_method_type;
				}
				break;
			case CONSTANT_InvokeDynamic:
				{
					cp_invoke_dynamic * call_site_spec = (cp_invoke_dynamic*)info;
					invoke_dynamic_ref * call_site = memery::alloc_meta<invoke_dynamic_ref>();
					call_site->boostrap_method_attr_index = call_site_spec->boostrap_method_attr_index;
					cp_name_and_type * name_and_type = get<cp_name_and_type>(call_site_spec->name_and_type_index);
					call_site->name = get<symbol>(name_and_type->name_index);
					call_site->discriptor = get<symbol>(name_and_type->discriptor_index);
					ret->sym_invoke_dynamic = call_site;
				}
				break;
		}
		rt_pool->put(ret, index);
		return ret;
	}
};


void classloader::load_jar(const std::string & path, thread * current_thread)
{
	zip * rt_jar = zip_open(path.c_str(), 0, nullptr);
	if (!rt_jar) return;

	int count = zip_get_num_files(rt_jar);
	int buf_size = 1024;
	char * buf = (char*)malloc(buf_size);
	for (int i = 0 ; i < count; i ++) {
		struct zip_stat stat;
		if (zip_stat_index(rt_jar, i, ZIP_FL_UNCHANGED, &stat) != 0) continue;
		if (buf_size < stat.size) {
			char * new_buf = (char*)realloc(buf, stat.size);
			if (!new_buf)  continue;
			buf = new_buf;
			buf_size = stat.size;
		}
		zip_file * class_file = zip_fopen_index(rt_jar, i, ZIP_FL_UNCHANGED);
		if (!class_file) {
			continue;
		}

		int rc = zip_fread(class_file, buf, stat.size);
		zip_fclose(class_file);
		if (rc < 1) {
			continue;
		}
		byte_stream stream;
		stream.set_buf(buf, stat.size);
		load_class(stream, current_thread);
	}
	zip_close(rt_jar);
	free(buf);
}
claxx * classloader::load_from_file(const std::string & file, thread * current_thread)
{
	int fd = open(file.c_str(), O_RDONLY);
	if (fd == -1) {
		return nullptr;
	}

	struct stat info;
	if (fstat(fd, &info) == -1) {
		close(fd);
		return nullptr;
	}

	if (!info.st_size) {
		close(fd);
		return nullptr;
	}

	char * buf = new char[info.st_size]();
	if (!buf) {
		close(fd);
		return nullptr;
	}

	int rd = 0;
	int left = info.st_size;
	while (left) {
		rd = ::read(fd, buf + info.st_size - left, left);
		if (rd == -1) {
			perror("read");
			close(fd);
			return nullptr;
		}
		if (!rd) break;
		left -= rd;
	}
	byte_stream stream;
	stream.set_buf(buf, info.st_size);
	return load_class(stream, current_thread);
}

claxx * classloader::load_class(const std::string & name, thread * current_thread)
{
	if (!name.length()) return nullptr;
	if (loaded_classes.find(name) != loaded_classes.end()) return loaded_classes[name];
	if (name[0] == '[') {
		claxx * ret = create_array_claxx(name, current_thread);
		if (ret) {
			return loaded_classes[name] = ret;
		}
	}
	std::string file_name = name + ".class";
	claxx * ret = load_from_file(file_name, current_thread);
	if (ret) return ret;

	if (jars.empty()) return nullptr;

	/*
	   int count = zip_get_num_files(rt_jar);
	   for (int i = 0 ; i < count; i ++) {
	   printf("%s\n", zip_get_name(rt_jar, i, ZIP_FL_UNCHANGED));
	   }
	   */

	struct zip_stat stat;
	zip * jar = nullptr;
	for (auto j : jars) {
		if (zip_stat(j, file_name.c_str(), ZIP_FL_UNCHANGED, &stat) == 0) {
			jar = j;
			break;
		}
	}
	if (!jar) return nullptr;
	
	char * buf = new char[stat.size];
	zip_file * class_file = zip_fopen(jar,file_name.c_str(), ZIP_FL_UNCHANGED);
	if (!class_file) {
		delete [] buf;
		//zip_close(rt_jar);
		return nullptr;
	}

	int rc = zip_fread(class_file, buf, stat.size);
	//printf("read %d/%d\n", rc, stat.size);
	byte_stream stream;
	stream.set_buf(buf, stat.size);
	zip_fclose(class_file);
	ret = load_class(stream, current_thread);
	delete [] buf;
	if (!ret) {
		throw std::runtime_error("classfile not found");
	}
	return ret;
}

claxx * classloader::load_class(byte_stream & stream, thread * current_thread)
{
	if (!stream.value()) return nullptr;

	claxx * java_class = memery::alloc_meta<claxx>();
	if (!java_class) return java_class;
	java_class->loader = this;

	//1.magic
	u4 magic = stream.get<u4>();
	if (magic != 0xcafebabe) {
		printf("not a class file %x\n", magic);
		return nullptr;
	}
	//2.version
	u2 minor_v = stream.get<u2>();
	u2 major_v = stream.get<u2>();


	std::vector<cp_info*> raw_pool_entries;
	java_class->cpool = parse_const_pool(stream, raw_pool_entries, current_thread);
	java_class->cpool->owner = java_class;
	if (!java_class->cpool) {
		memery::dealloc_meta(java_class);
		return nullptr;
	}
	java_class->cpool->owner = java_class;
	const raw_const_pool raw_pool(raw_pool_entries, nullptr, current_thread);


	java_class->access_flag = stream.get<u2>();
	java_class->name = java_class->cpool->get(stream.get<u2>())->sym_class->name;
	//fprintf(stderr, "%s const pool size %ld\n", java_class->name->c_str(), java_class->cpool->count);
	record_claxx(java_class);
	u2 super_index = stream.get<u2>();
	//printf("super %d\n", super_index);
	if (super_index) 
		java_class->super_class = java_class->cpool->get_class(super_index, current_thread);
	u2 interfece_count = stream.get<u2>();
	while (interfece_count --) {
		u2 interface_index = stream.get<u2>();
		java_class->interfaces.push_back(java_class->cpool->get_class(interface_index, current_thread));
	}
	u2 field_count = stream.get<u2>();

	//先把父类属性放进来,不知道对不对，但是肯定可以,owner怎么置，或者复制一个？,不用了生成了
	int mem_off = 0;
	int static_off = 0;
	if (java_class->super_class) {
		for (auto f : java_class->super_class->fields) {
			java_class->fields.insert(f);
		}
		for (auto f : java_class->super_class->static_fields) {
			java_class->static_fields.insert(f);
		}
		mem_off = java_class->super_class->member_size;
		static_off = java_class->super_class->static_size();;
	}
	
	for (int i = 0; i < field_count; i++) {
		field * f = memery::alloc_meta<field>();
		f->owner = java_class;
		f->access_flag = stream.get<u2>();
		f->name = raw_pool.get<symbol>( stream.get<u2>());
		f->discriptor = raw_pool.get<symbol>( stream.get<u2>());
		f->type = type_of_disc(f->discriptor->at(0));
		if (f->type == T_OBJECT) {
			//printf("field dis %s\n", f->discriptor->c_str());
			//f->meta = load_class_from_disc(f->discriptor->c_str(), current_thread);
		}else {
			f->meta = load_class(f->discriptor->c_str(), current_thread);
		}
		u2 attr_count = stream.get<u2>();
		while (attr_count --) {
			attribute * attr = parse_attribute(stream, raw_pool);
			if (attr) f->attributes.push_back(attr);
			else {
				//printf("parsing filed attr failed\n");
			}
		}
		int & off = f->is_static()? static_off : mem_off;
		if (f->is_static()) {
			off = static_off;
			java_class->static_fields[f->name->c_str()] = f;
		}else {
			java_class->fields[f->name->c_str()] = f;
		}
		//printf("field %s:%s static:%d type:%d size:%d\n", f->name->c_str(), f->discriptor->c_str(), f->is_static(),f->type, type_size[f->type-T_BOOLEAN]);
		
		f->offset = off;
		if (f->name->equals("reflectionData")) {
			//throw std::runtime_error(java_class->name->c_str());
		}
		off += type_size[f->type];
	}
	java_class->member_size = mem_off;
	java_class->static_member_size = static_off;
	//printf("%s mem size:%ld static mem size:%ld\n", java_class->name->c_str(), java_class->member_size, java_class->static_member_size);
	//printf("field count: %d\n",java_class->fields.size());

	u2 method_count = stream.get<u2>();
	java_class->method_by_index.resize(method_count);
	while (method_count --) {
		method * m = memery::alloc_meta<method>();
		m->owner = java_class;
		m->access_flag = stream.get<u2>();
		m->name = raw_pool.get<symbol>(stream.get<u2>());
		m->discriptor = raw_pool.get<symbol>(stream.get<u2>());
		//if (!m->is_static()) m->arg_types.push_back(T_OBJECT); //this
		m->ret_type = type_of_method_disc(m->discriptor, m->arg_types, m->param_types, m->arg_space);

		//for slot suport
		m->slot = method_count;
		java_class->method_by_index[method_count] = m;

		//printf("method native:%d %s:%s arg count %ld\n", m->is_native(), m->name->c_str(), m->discriptor->c_str(), m->arg_types.size());
		u2 attr_count = stream.get<u2>();
		while (attr_count --) {
			attribute * attr = parse_attribute(stream, raw_pool);
			if (attr) m->attributes.push_back(attr);
			else {
				printf("parsing method attr failed\n");
			}
		}
		java_class->methods[m->name->c_str()][m->discriptor->c_str()] = m;
	}
	//printf("method count: %d\n",java_class->methods.size());
	u2 attr_count = stream.get<u2>();
	while (attr_count --) {
		attribute * attr = parse_attribute(stream, raw_pool);
		if (attr) java_class->attributes.push_back(attr);
	}	

	for (auto info : raw_pool_entries) {
		if (info && info->tag != CONSTANT_Utf8) {
			memery::dealloc_meta(info);
		}
	}
	java_class->state = LINKED;
	//printf("loaded %s\n", java_class->name->c_str());
	create_mirror(java_class, current_thread);
	return java_class;
}


const_pool * classloader::parse_const_pool(byte_stream & stream, std::vector<cp_info*> & raw_pool, thread * current_thread)
{
	if (!stream.value()) return nullptr;

	const_pool * cpool = (const_pool*)memery::alloc_meta_space(sizeof(const_pool));
	u2 const_count = stream.get<u2>();
	new (cpool) const_pool(const_count);

	raw_pool.push_back(nullptr);
	for (int i = 1; i < const_count && stream.value(); i++) {
		u1 tag = stream.get<u1>();
		//printf("#%d [%d]\n", i,tag);
		cp_info * info = nullptr;
		switch (tag) {
			case CONSTANT_Class: 
				{
					cp_class * class_sym = memery::alloc_meta<cp_class>();
					class_sym->name_index = stream.get<u2>();
					info = class_sym;
				}
				break;
			case CONSTANT_FieldRef:
				{
					cp_field_ref * ref = memery::alloc_meta<cp_field_ref>();
					ref->class_index = stream.get<u2>();
					ref->name_and_type_index = stream.get<u2>();
					info = ref;
				}
				break;
			case CONSTANT_MethodRef:
				{
					cp_method_ref * ref = memery::alloc_meta<cp_method_ref>();
					ref->class_index = stream.get<u2>();
					ref->name_and_type_index = stream.get<u2>();
					info = ref;
				}
				break;
			case CONSTANT_InterfaceMethodRef:
				{
					cp_interface_method_ref * ref = memery::alloc_meta<cp_interface_method_ref>();
					ref->class_index = stream.get<u2>();
					ref->name_and_type_index = stream.get<u2>();
					info = ref;
				}
				break;
			case CONSTANT_String:
				{
					cp_string * string = memery::alloc_meta<cp_string>();
					string->string_index = stream.get<u2>();
					info = string;
				}
				break;
			case CONSTANT_Integer:
				{
					cp_int * num4 = memery::alloc_meta<cp_int>();
					num4->bytes = stream.get<u4>();
					info = num4;
				}
				break;
			case CONSTANT_Float:
				{
					cp_float * num4 = memery::alloc_meta<cp_float>();
					num4->bytes = stream.get<u4>();
					info = num4;
				}
				break;
			case CONSTANT_Long:
				{
					cp_long * num8 = memery::alloc_meta<cp_long>();
					u8 h = stream.get<u4>();
					u4 l = stream.get<u4>();
					num8->bytes = (h<<32) | l;
					info = num8;
				}
				break;
			case CONSTANT_Double:
				{
					cp_double * num8 = memery::alloc_meta<cp_double>();
					u8 h = stream.get<u4>();
					u4 l = stream.get<u4>();
					num8->bytes = (h<<32) | l;
					info = num8;
				}
				break;
			case CONSTANT_NameAndType:
				{
					cp_name_and_type * name_type = memery::alloc_meta<cp_name_and_type>();
					name_type->name_index = stream.get<u2>();
					name_type->discriptor_index = stream.get<u2>();
					info = name_type;
				}
				break;
			case CONSTANT_Utf8:
				{
					u2 length = stream.get<u2>();
					cp_utf8 * utf8 = (cp_utf8*)memery::alloc_meta_space(sizeof(cp_utf8) + length + 1);
					utf8->length = length;
					stream.read(utf8->bytes, length);
					symbol * h = symbol_pool::instance().instance().instance().put(utf8);
					if (h != utf8) {
						memery::dealloc_meta_space(utf8);
					}
					//printf("read utf8:%s\n",utf8->bytes);
					info = h;
				}
				break;
			case CONSTANT_MethodHandle:
				{
					cp_method_handle * method_handle = memery::alloc_meta<cp_method_handle>();
					method_handle->reference_kind = stream.get<u1>();
					method_handle->reference_index = stream.get<u2>();
					info = method_handle;
				}
				break;
			case CONSTANT_MethodType:
				{
					cp_method_type * method_type = memery::alloc_meta<cp_method_type>();
					method_type->discriptor_index = stream.get<u2>();
					info = method_type;
				}
				break;
			case CONSTANT_InvokeDynamic:
				{
					cp_invoke_dynamic * invoke = memery::alloc_meta<cp_invoke_dynamic>();
					invoke->boostrap_method_attr_index = stream.get<u2>();
					invoke->name_and_type_index = stream.get<u2>();
					info = invoke;
				}
				break;
			default: 
				{
					printf("[%d] unkown constant pool entry with tag=%d\n", i, tag);
					/*
					for (cp_info * c : raw_pool) {
						if(!c) continue;
						if (c->tag == CONSTANT_Utf8) 
							memery::dealloc_meta_space(c);
						else 
							memery::dealloc_meta(c);
					}
					return nullptr;
					*/
				}
		}
		if (info) {
			info->tag = tag;
			raw_pool.push_back(info);
			if (info->tag == CONSTANT_Long || info->tag == CONSTANT_Double) {
				raw_pool.push_back(nullptr);
				i ++;
			}
		}
	}

	raw_const_pool parser(raw_pool, cpool, current_thread);
	for (int i = 1; i < const_count; i ++) {
		parser.parse_entry(i);
	}
	return cpool;
}

verification_type_info classloader::parse_verification_type_info(byte_stream & stream)
{
	verification_type_info info;
	info.tag = stream.get<u1>();
	switch (info.tag) {
		case ITEM_Top:
		case ITEM_Integer:
		case ITEM_Float:
		case ITEM_Double:
		case ITEM_Long:
		case ITEM_Null:
		case ITEM_UninitializedThis:
			break;
		case ITEM_Object:
			info.cpool_index = stream.get<u2>();
			break;
		case ITEM_Uninitialized:
			info.offset = stream.get<u2>();
			break;
	}
	return info;
}

attribute * classloader::parse_attribute(byte_stream & stream,const raw_const_pool & cpool)
{
	symbol * name = cpool.get<symbol>( stream.get<u2>());
	if (!name) {
		return nullptr;
	}

	//printf("attr : %s\n", name->bytes);
	attribute * ret = nullptr;
	stream.get<u4>();
	if (name->equals("ConstantValue")) {
		constant_attr * constvalue = memery::alloc_meta<constant_attr>();
		constvalue->constantvalue_index = stream.get<u2>();
		ret = constvalue;
	}else if (name->equals("Code")) {
		code_attr * code = memery::alloc_meta<code_attr>();
		code->max_stacks = stream.get<u2>();
		code->max_locals = stream.get<u2>();
		code->code_length = stream.get<u4>();
		code->code = (u1*)memery::alloc_meta_space(code->code_length);
		if (code->code && stream.value() >= code->code_length) {
			stream.read(code->code, code->code_length);
		}

		u2 etable_length = stream.get<u2>();
		while (etable_length --) {
			exception e = {
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
			};
			code->exceptions.push_back(e);
		}
		u2 attr_count = stream.get<u2>();
		while (attr_count --) {
			attribute * attr = parse_attribute(stream, cpool);
			if (attr) code->attributes.push_back(attr);
		}
		ret = code;
	}else if (name->equals("StackMapTable")) {
		u2 count = stream.get<u2>();
		stack_map_table_attr * st_map_table = memery::alloc_meta<stack_map_table_attr>();
		while (count --) {
			stack_map_frame frame;
			u1 frame_type = stream.get<u1>();
			if (0 <= frame_type && frame_type <= 63) {
				frame.frame_type = SAME;
			}else if (64 <= frame_type && frame_type <= 127) {
				frame.frame_type = SAME_LOCALS_1_STACK_ITEM;
				frame.stack.push_back( parse_verification_type_info( stream));
			}else if (frame_type == 247) {
				frame.frame_type = SAME_LOCALS_1_STACK_ITEM_EXTENDED;
				frame.offset_delta = stream.get<u2>();	
				frame.stack.push_back( parse_verification_type_info( stream));
			}else if (248 <= frame_type && frame_type <= 250) {
				frame.frame_type = CHOP;
				frame.offset_delta = stream.get<u2>();	
			}else if (frame_type == 251) {
				frame.frame_type = SAME_FRAME_EXTENDED;
				frame.offset_delta = stream.get<u2>();	
			}else if (252 <= frame_type && frame_type <= 254) {
				frame.frame_type = APPEND;
				frame.offset_delta = stream.get<u2>();	
				for (int i = 0; i < frame_type - 251; i ++) {
					frame.locals.push_back( parse_verification_type_info( stream));
				}
			}else if (frame_type == 255){
				frame.frame_type = FULL_FRAME;
				frame.offset_delta = stream.get<u2>();	
				u2 nlocals = stream.get<u2>();
				while (nlocals --) {
					frame.locals.push_back( parse_verification_type_info( stream));
				}
				u2 nstack = stream.get<u2>();
				while (nstack --) {
					frame.stack.push_back( parse_verification_type_info( stream));
				}

			}else continue;
			st_map_table->entries.push_back(frame);
			ret = st_map_table;
		}
	}else if (name->equals("Exceptions")) {
		exceptions_attr * etable = memery::alloc_meta<exceptions_attr>();
		u2 count = stream.get<u2>();
		while (count --) {
			etable->exception_index_table.push_back(stream.get<u2>());
		}
		ret = etable;
	}else if (name->equals("InnerClasses")) {
		inner_classes_attr * inclasses = memery::alloc_meta<inner_classes_attr>();
		u2 count = stream.get<u2>();
		while (count --) {
			inner_class ic = {
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
			};
			inclasses->classes.push_back(ic);
		}
		ret = inclasses;
	}else if (name->equals("EnclosingMethod")) {
		enclosing_method_attr * enclosing = memery::alloc_meta<enclosing_method_attr>();
		enclosing->class_index = stream.get<u2>();
		enclosing->method_index = stream.get<u2>();
		ret = enclosing;
	}else if (name->equals("Signature")){
		signature_attr * sig = memery::alloc_meta<signature_attr>();
		sig->signature_index = stream.get<u2>();
		ret = sig;
	}else if (name->equals("SourceFile")) {
		source_attr * source = memery::alloc_meta<source_attr>();
		source->sourcefile_index = stream.get<u2>();
		ret = source;
	}else if (name->equals("LineNumberTable")) {
		line_number_table_attr * ln_table = memery::alloc_meta<line_number_table_attr>();
		u2 table_length = stream.get<u2>();
		while (table_length --) {
			u2 v1 = stream.get<u2>();
			u2 v2= stream.get<u2>();
			auto p = std::make_pair(v1, v2);
			//printf("line %d %d\n", p.first, p.second);
			ln_table->line_number_table.insert(p);
		}
		ret = ln_table;
	}else if (name->equals("LocalVariableTable")) {
		local_variable_table_attr * locals = memery::alloc_meta<local_variable_table_attr>();
		u2 count = stream.get<u2>();
		while (count --) {
			local_variable local = {
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
			};
			locals->local_variable_table.push_back(local);
		}
		ret = locals;
	}else if (name->equals("LocalVariableTypeTable")) {
		local_variable_type_table_attr * locals = memery::alloc_meta<local_variable_type_table_attr>();
		u2 count = stream.get<u2>();
		while (count --) {
			local_variable local = {
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
				stream.get<u2>(),
			};
			locals->local_variable_type_table.push_back(local);
		}
		ret = locals;
	}else if (name->equals("Deprecated")) {
		ret = memery::alloc_meta<deprecated_attr>();
	}else if (name->equals("RuntimeVisibleAnnotations")) {
		runtime_visible_anotations_attr * rvas = memery::alloc_meta<runtime_visible_anotations_attr>();
		u2 ancount = stream.get<u2>();
		while (ancount --) {
			rvas->anotation_ptrs.push_back(parse_annotation(stream));
		}
		ret = rvas;
	}else if (name->equals("AnnotationDefault")) {
		annotation_default_attr * ann = memery::alloc_meta<annotation_default_attr>();
		ann->default_value = parse_element_value(stream);
		ret = ann;
	}else if (name->equals("BootstrapMethods")) {
		bootstrap_methods_attr * bs = memery::alloc_meta<bootstrap_methods_attr>();
		u2 count = stream.get<u2>();
		while (count--) {
			bootstrap_method m;
			m.bootstrap_method_ref = stream.get<u2>();
			u2 acount = stream.get<u2>();
			while (acount--) {
				m.bootstrap_arguments.push_back(stream.get<u2>());
			}
			bs->bootstrap_methods.push_back(m);
		}
		ret = bs;
	}else {
		printf("unsuport attribute %s\n", name->c_str());
	}
	if (ret) {
		ret->name = name;
	}
	return ret;
}

element_value * classloader::parse_element_value(byte_stream & stream)
{
	element_value * value = memery::alloc_meta<element_value>();
	value->tag = stream.get<u1>();
	switch (value->tag) {
		case 'B':
		case 'C':
		case 'D':
		case 'F':
		case 'I':
		case 'J':
		case 'S':
		case 'Z':
		case 's':
			value->const_value_index = stream.get<u2>();
			break;
		case 'e':
			value->enum_const_value.type_name_index = stream.get<u2>();
			value->enum_const_value.const_name_index = stream.get<u2>();
			break;
		case 'c':
			value->class_info_index = stream.get<u2>();
			break;
		case '@':
			value->annotation_ptr = parse_annotation(stream); 
			break;
		case '[':
			{
				u2 num_values = stream.get<u2>();
				while (num_values --) {
					value->array_value.push_back(parse_element_value(stream));
				}
			}
			break;
		default:
			memery::dealloc_meta(value);
			value = nullptr;
			printf("unkown tag %c\n",value->tag);
			break;
	}
	return value;

}

annotation * classloader::parse_annotation(byte_stream & stream)
{
	annotation * ann = memery::alloc_meta<annotation>();
	ann->type_index = stream.get<u2>();
	u2 element_value_count = stream.get<u2>();
	while (element_value_count --) {
		annotation::element_value_pair pair;
		pair.element_name_index = stream.get<u2>();
		pair.value = parse_element_value(stream);
		ann->element_value_pairs.push_back(pair);
	}
	return ann;
}
		
jtype classloader::type_of_disc(char c)
{
	switch (c) {
		case 'B':
			return T_BYTE;
		case 'C':
			return T_CHAR;
		case 'S': 
			return T_SHORT;
		case 'Z':
			return T_BOOLEAN;
		case 'V':
			return T_VOID;
		case 'F':
			return T_FLOAT;
		case 'J':
			return T_LONG;
		case 'D':
			return T_DOUBLE;
		case 'L':
			return T_OBJECT;
		case '[':
			return T_OBJECT;
		default:
			return T_INT;
	}
}

claxx * classloader::load_class_from_disc(const std::string & disc, thread * current_thread)
{
	if (disc.length() < 3) return nullptr;
	return load_class(std::string(disc, 1, disc.length() - 2), current_thread);
}

jtype classloader::type_of_method_disc(symbol * dis,std::vector<jtype> & args, std::vector<symbol*> & param_types, uint32_t & space)
{
	int i;
	for (i = 1 ; i < dis->length-1; i ++) {
		if (dis->at(i) == '(') continue;
		if (dis->at(i) == ')') {
			i++;
			break;
		}
		jtype t = type_of_disc(dis->at(i));
		space ++;
		if (type_size[t] > sizeof(jint)) space ++;
		args.push_back(t);
		std::string name;
		if (dis->at(i) == 'L'){
			int s = i + 1;
			while (dis->at(i) != ';') i++;
			name = std::string(dis->c_str(), s, i - s);
		}else if (dis->at(i) == '['){
			int s = i;
			while (dis->at(i) == '[') i ++;
			if (dis->at(i) == 'L') {
				while (dis->at(i) != ';') i++;
			}
			name = std::string(dis->c_str(), s, i - s + 1);
		}else {
			name = std::string(dis->c_str(), i, 1);
		}
		param_types.push_back(symbol_pool::instance().put(name));
		
	}
#if 0
	if (dis->equals("(Ljava/lang/Class;[Ljava/lang/Class;[Ljava/lang/Class;IILjava/lang/String;[B[B)V")) {
		getchar();
	}
#endif
	return type_of_disc(dis->at(i));
}

void classloader::record_claxx(claxx * c)
{
	if (!c) return;
	loaded_classes[c->name->c_str()] = c;
}

void classloader::link_class(claxx * to_link)
{
}

void classloader::initialize_class(claxx * to_init, thread * current_thread)
{
	if (!to_init || !current_thread) return;
	if (to_init->state >= INITING) return;
	if (to_init->super_class && to_init->super_class->state < INITED) initialize_class(to_init->super_class, current_thread);
	to_init->static_obj = memery::alloc_heap_object(to_init, true);
	to_init->state = INITING;
	method * clinit = to_init->get_clinit_method();
	if (clinit) {
		current_thread->call(clinit);
	}
	to_init->state = INITED;
}
		
void classloader::create_mirror(claxx * cls, thread * current_thread)
{
	log::debug("create mirror for %s", cls->name->c_str());
	claxx * java_lang_Class = load_class("java/lang/Class", current_thread);
	cls->mirror = memery::alloc_heap_object(java_lang_Class);
	/*
	jreference name = current_thread->get_env()->create_string(cls->name->c_str());
	auto nfd = java_lang_Class->lookup_field("name");
	current_thread->get_env()->set_object_field(cls->mirror, nfd, name);
	*/
	mirror_classes[cls->mirror] = cls;
}

claxx * classloader::claxx_from_mirror(jreference m)
{
	auto it = mirror_classes.find(m);
	if ( it != mirror_classes.end()) {
		return it->second; 
	}
	return nullptr;
}
array_claxx * classloader::create_array_claxx(const std::string & name, thread * current_thread)
{
	if (name[0] != '[') return nullptr;
	if (name.length() < 2) return nullptr;
	array_claxx * ret = new array_claxx(name, this, current_thread);
	if (ret)
		loaded_classes[name] = ret;
	return  ret;
}
	
jreference classloader::create_primitive(jtype t)
{
	return (new primitive_claxx(t, this))->mirror;
}
