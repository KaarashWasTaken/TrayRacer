#pragma once
#include <atomic>
#include <iostream>
using std::atomic;
using std::cout;
using std::endl;

//custom spinlock function
class Spinlock
{
public:
	static bool testAndSet(atomic<int>& flag)
	{
		int lockResult = flag.fetch_add(1, std::memory_order_acquire);
		//check if the old value was 0 which means the lock was free
		return lockResult == 0;
	}
	void lock()
	{
		while (testAndSet(lockFlag))
		{
#ifdef _WIN32 
#pragma pause
#else
			// Optional: Short sleep (use with caution)
			std::this_thread::yield();
#endif
		}
	}
	void unlock()
	{
		lockFlag.store(0, std::memory_order_release);
	}
private:
	atomic<int> lockFlag{ 0 };
};