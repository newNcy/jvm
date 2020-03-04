#pragma once

class claxx;

struct member_operator 
{
	private:
		char * start = nullptr;
		size_t length = 0;
	public:
		member_operator(char * data_start, size_t data_length): start(data_start), length(data_length) {}
		template <typename T> 
		T get(int offset)
		{
			if (offset < 0 || offset >= length) abort();
			return *(T*)(start + offset);
		}

		template <typename T>
		void put(T t, int offset)
		{
			if (offset < 0 || offset >= length) abort();
			*(T*)(start + offset) = t;
		}
};

struct object
{
	claxx * meta = nullptr;
	char * static_data = nullptr;
	char data[];
};
