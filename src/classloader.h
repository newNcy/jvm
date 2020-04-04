#pragma once
#include "class.h"
#include "thread.h"
#include <zip.h>

class raw_const_pool;
class classloader
{
	friend class jvm;
	public:
		std::string runtime_path;
		claxx * load_class(const std::string & name, thread * current_thread);
		claxx * load_class(byte_stream & stream, thread * current_thread);
		claxx * load_class_from_disc(const std::string & disc, thread * current_thread);
		array_claxx * create_array_claxx(const std::string & name, thread * current_thread);
		void load_jar(const std::string & path, thread * current_thread);
		claxx *  load_from_file(const std::string & file, thread * current_thread);
		jtype type_of_disc(char c);
		jtype type_of_method_disc(symbol * dis,std::vector<jtype> & args);

		void link_class(claxx * to_link);
		void initialize_class(claxx * to_init,thread *  current_thread);
	private:
		
		const_pool * parse_const_pool(byte_stream & stream, std::vector<cp_info*> &, thread * current_thread);
		attribute * parse_attribute(byte_stream & stream,const raw_const_pool & cpool);
		annotation * parse_annotation(byte_stream & stream);
		verification_type_info parse_verification_type_info(byte_stream & stream);
		element_value * parse_element_value(byte_stream & stream);
		std::map <std::string, claxx *> loaded_classes;
		//thread * current_thread = nullptr;
		zip * rt_jar = nullptr;
};
