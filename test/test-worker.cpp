#define BOOST_TEST_MODULE test_worker

#include "testHelpers.h"
#include <boost/test/unit_test.hpp>
#include <set>
#include <stream_support.hpp>
#include <worker.h>

uint32_t
workCounter()
{
	static std::atomic_uint32_t n;
	usleep(1000);
	return n++;
}

BOOST_AUTO_TEST_CASE(basic_slow_counter)
{
	std::vector<Worker::WorkPtrT<uint32_t>> ps;
	for (int i {}; i < 30; ++i) {
		ps.push_back(Worker::addWork(workCounter));
	}
	std::set<uint32_t> out;
	std::transform(ps.begin(), ps.end(), std::inserter(out, out.end()), [](auto && p) {
		return p->get();
	});
	BOOST_REQUIRE_EQUAL(out.size(), ps.size());
	BOOST_CHECK_EQUAL(*out.begin(), 0);
	BOOST_CHECK_EQUAL(*out.rbegin(), ps.size() - 1);
}

BOOST_AUTO_TEST_CASE(basic_error_handler)
{
	auto workitem = Worker::addWork([]() {
		throw std::runtime_error {"test"};
	});
	BOOST_CHECK_THROW(workitem->get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(basic_void_work)
{
	auto workitem = Worker::addWork([]() {});
	BOOST_CHECK_NO_THROW(workitem->get());
}
