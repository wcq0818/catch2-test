#include <catch.hpp>
#include <stdio.h>
#include <boost\thread\mutex.hpp>  
#include <boost\thread\recursive_mutex.hpp>  
#include <boost\thread\lock_guard.hpp>

namespace 
{
	boost::mutex g_mutex;

	void threadfun1()
	{
		printf("enter threadfun1...\n");
		boost::lock_guard<boost::mutex> lock(g_mutex);
		printf("execute threadfun1...\n");
	}

	void threadfun2()
	{
		printf("enter threadfun2...\n");
		boost::lock_guard<boost::mutex> lock(g_mutex);
		threadfun1();
		printf("execute threadfun2...\n");
	}
}

namespace 
{
	boost::recursive_mutex g_rec_mutex;

	void threadfun3()
	{
		printf("enter threadfun3...\n");
		boost::recursive_mutex::scoped_lock lock(g_rec_mutex);
		// 当然这种写法也可以
		// boost::lock_guard<boost::recursive_mutex> lock(g_rec_mutex);
		printf("execute threadfun3...\n");
	}

	void threadfun4()
	{
		printf("enter threadfun4...\n");
		boost::recursive_mutex::scoped_lock lock(g_rec_mutex);
		threadfun3();
		printf("execute threadfun4...\n");
	}
}

// 死锁的例子
TEST_CASE("test_thread_deadlock", "test_thread_deadlock[.hide]")
{
	threadfun2();
}

// 利用递归式互斥量来避免这个问题
TEST_CASE("test_thread_recursivelock", "test_thread_recursivelock")
{
	threadfun4();
}