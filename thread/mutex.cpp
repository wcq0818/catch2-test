#include <catch.hpp>
#include <string.h>
#include <stdio.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex, std::unique_lock
#include <vector>
#include <chrono>         // std::chrono::milliseconds
#include <thread>         // std::thread
#include <mutex>          // std::timed_mutex, std::unique_lock, std::defer_lock
#include <queue>
#include <list>
#include <boost\ref.hpp>  
#include <boost\thread.hpp>
#include "boost\bind.hpp"
#include "thread_pool.hpp"

int g_num = 0;      //全局变量，写者改变该全局变量，读者读该全局变量  
boost::shared_mutex s_mtx;   //全局shared_mutex对象  

					 //写线程  
void read_(std::string str)
{
	const char* c = str.c_str();
	while (1)
	{
		{
			boost::shared_lock<boost::shared_mutex> m(s_mtx);    //读锁定，shared_lock  
			printf("线程%s进入临界区，global_num = %d\n", c, g_num);
			boost::this_thread::sleep(boost::posix_time::seconds(1));    //sleep 1秒，给足够的时间看其他线程是否能进入临界区  
			printf("线程%s离开临界区...\n", c);
		}
		boost::this_thread::sleep(boost::posix_time::seconds(10));   //读线程离开后再slpp 1秒（改变这个值可以看到不一样的效果）  
	}
}

//读线程  
void writer_(std::string str)
{
	const char* c = str.c_str();
	while (1)
	{
		{
			boost::unique_lock<boost::shared_mutex> m(s_mtx);    //写锁定，unique_lock  
			++g_num;
			printf("线程%s进入临界区，global_num = %d\n", c, g_num);
			boost::this_thread::sleep(boost::posix_time::seconds(1));    //sleep 1秒，让其他线程有时间进入临界区  
			printf("线程%s离开临界区...\n", c);
		}
		boost::this_thread::sleep(boost::posix_time::seconds(10));   //写线程离开后再slpp 2秒，这里比读线程多一秒是因为如果这里也是1秒，那两个写线程一起请求锁时会让读线程饥饿  
	}
}

TEST_CASE("mutex", "mutex[.hide]")
{
// 	std::string r1 = "read_1";
// 	std::string r2 = "read_2";
// 	std::string w1 = "writer_1";
// 	std::string w2 = "writer_2";

	int cpu_core_num = boost::thread::hardware_concurrency();
	printf("cpu_core_num = %d\n", cpu_core_num);
	boost::thread_group tg;

	for (size_t i = 0; i < 100000; i++)
	{
		printf("create_thread %i ...\n", i);
		if (i % 10000 == 0)
		{
			std::string name = std::to_string(i);
			name = "write_" + name;
			tg.create_thread(bind(writer_, name/*boost::ref(name)*/));
		}
		else
		{
			if (i % 2000 == 0)
			{
				getchar();
			}
			std::string name = std::to_string(i);
			name = "read_" + name;
			tg.create_thread(bind(read_, name/*boost::ref(name)*/));
		}
	}

// 	tg.create_thread(bind(read_, boost::ref(r1)));   //两个读者  
// 	tg.create_thread(bind(read_, boost::ref(r2)));

// 	tg.create_thread(bind(writer_, boost::ref(w1)));  //两个写者  
// 	tg.create_thread(bind(writer_, boost::ref(w2)));

	tg.join_all();
}

boost::mutex mutex;
std::queue<int> data;

class HelloWorld
{
public:
	bool start;
	void prinf()
	{
		start = 1;

		while (start)
		{
			std::cout << "read begin ..." << std::endl;
			boost::mutex::scoped_lock lock(mutex);
			//boost::unique_lock<boost::mutex> lock(mutex);
			//boost::lock_guard<boost::mutex> lock(mutex);
			//lock.lock();
			while (!data.empty())
			{
				int i = data.front();
				std::cout << "the number is " << " " << i << std::endl;
				data.pop();
			}
			//lock.unlock();
			std::cout << "read over ..."<< std::endl;
		}
	}

	boost::mutex hello_mutex;

	void hello(const std::string& str)
	{
		printf("hello begin\n");
		boost::unique_lock<boost::mutex> lock(hello_mutex, boost::posix_time::seconds(1));
		printf("hello end\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		printf("hello over\n");
	}

	void hello2(const std::string& str)
	{
		printf("hello2 begin\n");
		boost::unique_lock<boost::mutex> lock(hello_mutex, boost::posix_time::seconds(1));
		printf("hello2 end\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		printf("hello2 over\n");
	}
};

TEST_CASE("mutex 2", "mutex 2[]")
{
	HelloWorld obj;
	boost::thread thrd(boost::bind(&HelloWorld::hello, &obj, "Hello World, I'm a thread!"));
	boost::thread thrd2(boost::bind(&HelloWorld::hello2, &obj, "Hello World, I'm a thread hello2!"));
	thrd.join();
	thrd2.join();
}

std::mutex mtx;           // mutex for critical section
std::once_flag flag;

void print_block(int n, char c) 
{
	printf("print_block\n");
	//unique_lock有多组构造函数, 这里std::defer_lock不设置锁状态
	std::unique_lock<std::mutex> my_lock(mtx, std::defer_lock);
	//std::lock_guard<std::mutex> my_lock(mtx);
	//尝试加锁, 如果加锁成功则执行
	//(适合定时执行一个job的场景, 一个线程执行就可以, 可以用更新时间戳辅助)
	if (my_lock.try_lock())
	{
		for (int i = 0; i < n * n * n; ++i)
		{
			//std::cout << c;
		}
// 		std::cout << '\n';
		printf("try_lock success\n");
	}
	else
	{
		printf("try_lock failed\n");
	}
}

void run_one(int &n) 
{
	//std::cout << std::time(nullptr) << std::endl;
	//printf("%d", std::time(nullptr));
	std::call_once(flag, [&n] {n = n + 1; }); //只执行一次, 适合延迟加载; 多线程static变量情况
}

TEST_CASE("mutex 3", "mutex 3[.hide]")
{
	std::vector<std::thread> ver;
	int num = 0;
	for (auto i = 0; i < 10; ++i) 
	{
		ver.emplace_back(print_block, 50, '*');
		ver.emplace_back(run_one, std::ref(num));
	}

	for (auto &t : ver) 
	{
		t.join();
	}
	std::cout << num << std::endl;
}

std::timed_mutex time_mtx;

void fireworks() 
{
	std::unique_lock<std::timed_mutex> lck(time_mtx, std::defer_lock);
	// waiting to get a lock: each thread prints "-" every 200ms:
// 	while (!lck.try_lock())
// 	{
// 		std::cout << "-";
// 		std::this_thread::sleep_for(std::chrono::milliseconds(200));
// 	}
	while (!lck.try_lock_for(std::chrono::milliseconds(0))) 
	{
		std::cout << "-";
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	// got a lock! - wait for 1s, then this thread prints "*"
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "*\n";
}

TEST_CASE("mutex 4", "mutex 4[.hide]")
{
	std::thread threads[10];
	// spawn 10 threads:
	for (int i = 0; i < 10; ++i)
	{
		threads[i] = std::thread(fireworks);
	}
	for (auto& th : threads)
	{
		th.join();
	}
}

std::mutex mtx5;           // mutex for critical section

void print_star() 
{
	std::unique_lock<std::mutex> lck(mtx5, std::try_to_lock);
	// print '*' if successfully locked, 'x' otherwise: 
	//if (lck)
	if (lck.owns_lock())
	{
		std::cout << '*';
	}
	else
	{
		std::cout << 'x';
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST_CASE("mutex 5", "mutex 5[.hide]")
{
	std::vector<std::thread> threads;
	for (int i = 0; i < 500; ++i)
	{
		threads.emplace_back(print_star);
	}
	for (auto& x : threads)
	{
		x.join();
	}
}

boost::shared_ptr<boost::recursive_mutex> action_task_mutexs_;

boost::recursive_mutex io_mutex;    

void run()  
{    
	io_mutex.lock();
	std::cout << "run lock..." << std::endl;
	io_mutex.lock();
	std::cout << "run lock..." << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(5)); 
	io_mutex.unlock();
	std::cout << "run unlock..." << std::endl;
	boost::this_thread::sleep(boost::posix_time::seconds(5));
	io_mutex.unlock();
	std::cout << "run unlock..." << std::endl;
} 

void run2()  
{    
	io_mutex.lock();
	std::cout << "run2 lock..." << std::endl;
	for (int i = 0; i < 10; ++i)    
	{    
		std::cout << i << std::endl;    
	}
	io_mutex.unlock();
	std::cout << "run2 unlock..." << std::endl;
} 

TEST_CASE("recursive_mutex", "recursive_mutex[.hide]")
{
	int get_count = 0;
	while (1)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1)); 
		int n = ((get_count++)*3+1);
		std::cout << n << std::endl;
	}

	boost::thread_group tg;
	tg.create_thread(run);
	boost::this_thread::sleep(boost::posix_time::seconds(1)); 
	tg.create_thread(run2);
	tg.join_all();
	getchar();
}

TEST_CASE("thread_pool", "thread_pool[.hide]")
{
	thread_pool thread_pool_(2);
	thread_pool_.start();
	for (int i = 0; i < 10000; i++)
	{
		getchar();
		thread_task_ptr task_ptr = boost::make_shared<thread_task>();
		thread_pool_.put(task_ptr);
	}
}