#pragma once
/*
 * monitor java monitor机制
 */

#include "class.h"
#include <cstdint>
#include <queue>
#include <mutex>
#include <condition_variable>
class monitor
{
	private:
		std::condition_variable enter_lock;
		std::mutex modify_lock; //确保操作都是互斥的
		thread * owner = nullptr;
		uint32_t entry_count = 0;
	public:
		void enter(thread * t);
		void exit(thread * t);
};
