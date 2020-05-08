#include "monitor.h"
#include "class.h"
#include <mutex>

void monitor::enter(thread * t) 
{
	std::unique_lock<std::mutex> _(modify_lock);
	enter_lock.wait(_, [this, t] { return entry_count == 0 || owner == t; });
	entry_count ++;
	owner = t;
	_.unlock();
}


void monitor::exit(thread * t) 
{
	std::lock_guard<std::mutex> _(modify_lock);
	if (owner != t) return;
	entry_count --;
	enter_lock.notify_one();
}
