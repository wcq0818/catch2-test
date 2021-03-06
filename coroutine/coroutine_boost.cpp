#include "catch.hpp"
#include <iostream>
#include <boost/coroutine2/all.hpp>

void foo_0(boost::coroutines2::coroutine<void>::pull_type & sink) 
{
	std::cout << "a ";
	sink();
	std::cout << "b ";
	sink();
	std::cout << "c ";
}

TEST_CASE("test_coroutine Run", "[test_coroutine]")
{
	boost::coroutines2::coroutine<void>::push_type source(foo_0);
	std::cout << "1 ";
	source();
	std::cout << "2 ";
	source();
	std::cout << "3 ";
	source();
	std::cout << std::endl;
}

void foo_1(boost::coroutines2::coroutine<std::string>::pull_type & sink)
{
	std::cout << "get " << sink.get() << " from main() by foo()\n";
	sink();
	std::cout << "get " << sink.get() << " from main() by foo()\n";
	sink();
}

TEST_CASE("test_coroutine_1 Run", "[test_coroutine_1]")
{
	std::string str1("HELLO");
	std::string str2("WORLD");
	boost::coroutines2::coroutine<std::string>::push_type source(foo_1);
	std::cout << "pass " << str1 << " from main() to foo()\n";
	source(str1);
	std::cout << "pass " << str2 << " from main() to foo()\n";
	source(str2);
}

constexpr int N = 10;

// 方法一：中规中矩
void foo(boost::coroutines2::coroutine<int>::pull_type & sink) 
{
	using coIter = boost::coroutines2::coroutine<int>::pull_type::iterator;
	for (coIter start = begin(sink); start != end(sink); ++start) 
	{
		std::cout << "retrieve " << *start << "\n";
	}
}
// 方法二：auto自动推导
void foo2(boost::coroutines2::coroutine<int>::pull_type & sink) 
{
	for (auto val : sink) 
	{
		std::cout << "retrieve " << val << "\n";
	}
}
// 方法三：守旧
void foo3(boost::coroutines2::coroutine<int>::pull_type & sink)
{
	for (int i = 0; i < N; i++) 
	{
		std::cout << "retrieve " << sink.get() << "\n";
		sink();
	}
}

TEST_CASE("test_coroutine_3 Run", "[test_coroutine_3]")
{
	boost::coroutines2::coroutine<int>::push_type source(foo2);
	for (int i = 0; i < N; i++) 
	{
		source(i);
	}
}

void cooperative(boost::coroutines2::coroutine<int>::push_type &sink, int i, int a, int b)
{
	int j = i;

	//调用main  
	sink(++j);

	//调用main  
	sink(++j);

	std::cout << "end\n";
}

TEST_CASE("test_cooperative Run", "[test_cooperative]")
{
	//传入一个参数，初始值为0  
	boost::coroutines2::coroutine<int>::pull_type source { std::bind(cooperative, std::placeholders::_1, 10, 20, 30) };
	std::cout << source.get() << '\n';

	//调用cooperative  
	source();
	std::cout << source.get() << '\n';

	//调用cooperative  
	source();
}

void cooperative_tuple(boost::coroutines2::coroutine<std::tuple<int, double, std::string>>::pull_type &source)
{
	auto args = source.get();
	std::cout << std::get<0>(args) << " " << std::get<1>(args) << " " << std::get<2>(args) << '\n';

	source();

	args = source.get();
	std::cout << std::get<0>(args) << " " << std::get<1>(args) << " " << std::get<2>(args) << '\n';
}

TEST_CASE("test_cooperative_tuple Run", "[test_cooperative_tuple]")
{
	boost::coroutines2::coroutine<std::tuple<int, double, std::string>>::push_type sink { cooperative_tuple };

	//通过tuple传递多个参数
	sink(std::make_tuple(0, 0.0, "aaa"));

	//通过tuple传递多个参数
	sink(std::make_tuple(1, 1.1, "bbb"));

	std::cout << "end\n";
}