#include "catch.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "boost/algorithm/string.hpp"
#include <boost/algorithm/string/split.hpp>
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index/hashed_index.hpp"
#include "boost/multi_index/identity.hpp"
#include "boost/multi_index/member.hpp"
#include "boost/multi_index/random_access_index.hpp"
#include "boost/multi_index/mem_fun.hpp"



//WordCnt类，记录了单词及对应的出现个数
struct WordCnt
{	std::string word;
	int cnt;
	int col;

	WordCnt(const std::string& word_, int cnt_, int col = 0) 
		: word(word_)
		, cnt(cnt_)
		, col(col)
	{
	}
	std::size_t word_length() const { return word.size(); }
	//默认按照cnt排序
	bool operator<(const WordCnt& rhs) const 
	{
		return cnt < rhs.cnt;
	}
};//WordCnt

struct by_word_cnt {};
struct by_member {};
struct by_col {};

typedef boost::multi_index::multi_index_container<
	//容器内元素类型
	WordCnt,
	boost::multi_index::indexed_by<
		boost::multi_index::ordered_non_unique<
			boost::multi_index::tag<by_word_cnt>,
			boost::multi_index::identity<WordCnt>
		>,
		//无序，也就是hash表，hashkey使用 WorkdCnt::word
// 		boost::multi_index::ordered_non_unique<
// 			boost::multi_index::tag<by_member>,
// 			boost::multi_index::member<WordCnt, std::string, &WordCnt::word>
// 		>,
		boost::multi_index::ordered_non_unique<
			boost::multi_index::tag<by_col>,
			boost::multi_index::member<WordCnt, int, &WordCnt::col>
		>
	>
> WordCntContainer;

TEST_CASE("word_count_sort", "[word_count_sort][.hide]")
{
	WordCntContainer word_cnt_container;
	const int size = 0;
	for (int i = size; i >= 0; i--)
	{
		for (int j = size; j >= 0; j--)
		{
			word_cnt_container.insert({ "A", i, j });
			word_cnt_container.insert({ "B", i, j });
			word_cnt_container.insert({ "C", i, j });
		}
	}
	for (const auto word_cnt : word_cnt_container)
	{
		std::cout << word_cnt.cnt << "\t" << word_cnt.word << "\t" << word_cnt.col << std::endl;
	}
}

TEST_CASE("word_count", "[word_count][.hide]")
{
	WordCntContainer word_cnt_container;

	std::vector<std::string> contents;
	std::string value = "hello hello hello hello hello cpp cpp cpp cpp go go go python python shell";
	//文本 -> 单词vec
	boost::split(contents, value, boost::is_any_of(" "));

	//取第二个索引，也就是hash表
// 	auto& word_index = word_cnt_container.get<1>();
// 	for (size_t i = 0; i < contents.size(); ++i)
// 	{
// 		const std::string& word = contents[i];
// 		auto iter = word_index.find(word);
// 		//存在则cnt+1，否则插入1
// 		if (iter == word_index.end()) 
// 		{
// 			word_index.insert({ word, 1 });
// 		}
// 		else
// 		{
// 			word_index.modify(iter, [](WordCnt& word_cnt) { word_cnt.cnt++; });
// 		}
// 	}

	//取word_cnt_container，即第一个索引，也就是按照cnt升序排列
	for (const auto word_cnt : word_cnt_container) 
	{
		//shell   1
		//python  2
		//go      3
		//cpp     4
		//hello   5
		std::cout << word_cnt.word << "\t" << word_cnt.cnt << std::endl;
	}
}

//Employee只有name一个成员函数
//name_length成员函数返回name的长度
struct Employee
{
	Employee(const std::string& name_) 
		:name(name_) 
	{
	}
	std::size_t name_length() const { return name.size(); }
	std::string name;
};//Employee

struct by_name_length {};
struct by_random_access {};

typedef boost::multi_index::multi_index_container<
	Employee,
	boost::multi_index::indexed_by<
	//第一个索引使用const_mem_fun定义，fun取Employee::name_length，也就是name的长度
		boost::multi_index::ordered_non_unique<
			boost::multi_index::tag<by_name_length>,
			boost::multi_index::const_mem_fun<Employee, std::size_t, &Employee::name_length>,
			//降序排列
			std::greater<std::size_t>
			>,
		//支持按照插入顺序随机读写
		boost::multi_index::random_access<
			boost::multi_index::tag<by_random_access>
		>
	>
> EmployeeSet;

TEST_CASE("EmployeeSet", "[EmployeeSet][.hide]")
{
	EmployeeSet employees;

	employees.insert({ "Jeff Dean" });
	employees.insert({ "Google" });
	employees.insert({ "Bidu" });
	employees.insert({ "Markus Heule" });
	employees.insert({ "Vlad Losev" });

	//ordered_non_unique按照name长度降序输出
	// Markus Heule
	// Vlad Losev
	// Jeff Dean
	// Google
	// Bidu
	for (const auto employee : employees.get<by_name_length>())
	{
		std::cout << employee.name << std::endl;
	}

	//random_access按照插入顺序顺序输出
	// Jeff Dean
	// Google
	// Bidu
	// Markus Heule
	// Vlad Losev
	std::cout << std::endl;
	for (const auto employee : employees.get<by_random_access>())
	{
		std::cout << employee.name << std::endl;
	}
	std::cout << std::endl;
	//random_access支持operator[] at capacity reserve等接口
	//Jeff Dean
	std::cout << employees.get<1>()[0].name << std::endl;
	//Google
	std::cout << employees.get<1>().at(1).name << std::endl;

}