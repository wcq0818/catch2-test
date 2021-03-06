#include "catch.hpp"
#include <iostream>
#include <ctime>
#include "boost/multi_index_container.hpp"//boost::multi_index::multi_index_container
#include "boost/multi_index/ordered_index.hpp"//insert
#include "boost/multi_index/member.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include <boost/variant.hpp>

template <typename... T>
using tupleType = std::tuple<T...>;

template <typename T>
class tupleClass
{
public:
	T value_;

public:
	tupleClass() = default;
	tupleClass(T value)
	{
		value_ = value;
	}
};

using testTupleType = tupleType<int, double, int>;
using testTupleClass = tupleClass<testTupleType>;

using testTupleMultiIndex = boost::multi_index::multi_index_container
<
	testTupleClass,
	boost::multi_index::indexed_by
	<
	boost::multi_index::ordered_non_unique<
	boost::multi_index::member<testTupleClass, testTupleType, &testTupleClass::value_>
	>
	>
>;

TEST_CASE("test_multi_index_sort", "[test_multi_index_sort]")
{
	testTupleMultiIndex my_container;

 	testTupleType my_data1{ 1, 1.0, 1 };
	testTupleType my_data2( 1, 1.0, 1 );
// 	testTupleClass my_class{ my_data };

	for (size_t i = 2; i > 0; i--)
	{
		for (size_t j = 1; j < 4; j++)
		{
			for (size_t k = 3; k > 0; k--)
			{
				my_container.insert({ { (int)k, (double)j, (int)i } });
			}
		}
	}
	for (const auto index : /*my_container.get<0>()*/boost::multi_index::get<0>(my_container))
	{
		//std::cout << std::get<0>(index.value_) << " " << std::get<1>(index.value_) << " " << std::get<2>(index.value_) << std::endl;
		
		//std::cout << i << " " << j << " " << k << std::endl;
	}
	boost::tuple<int, double> data;
	boost::get<0>(data);
	data.get<0>();

	testTupleType datas{ 1, 1.0, 1 };
	//const auto&[i, j, k] = datas;
}