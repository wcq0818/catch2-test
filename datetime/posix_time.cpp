#include <catch.hpp>
#include <stdio.h>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

TEST_CASE("posix_time", "posix_time[.hide]")
{
	boost::posix_time::ptime message_time = boost::posix_time::microsec_clock::local_time();
	std::string str_time = boost::posix_time::to_simple_string(message_time);

	char date_string[20];
	const auto ymd = message_time.date().year_month_day();
	const auto time = message_time.time_of_day();

	sprintf(date_string, "%04d-%02d-%02d %02d:%02d:%02d %03d", ymd.year, ymd.month.as_number(), ymd.day.as_number(), time.hours(), time.minutes(), time.seconds(), time.fractional_seconds() / 1000);

	printf("%s", date_string);
}
