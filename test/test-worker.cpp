#define BOOST_TEST_MODULE test_worker

#include "testHelpers.h"
#include <boost/test/unit_test.hpp>
#include <set>
#include <stream_support.h>
#include <worker.h>

uint32_t
workCounter()
{
	static std::atomic_uint32_t n;
	usleep(1000);
	return n++;
}

void
workVoid()
{
	usleep(1000);
}

void
workFail()
{
	usleep(1000);
	throw std::runtime_error {"test"};
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
	auto workitem = Worker::addWork(workFail);
	BOOST_CHECK_THROW(workitem->get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(basic_void_work)
{
	auto workitem = Worker::addWork(workVoid);
	BOOST_CHECK_NO_THROW(workitem->get());
}

BOOST_AUTO_TEST_CASE(lambda_void)
{
	BOOST_CHECK_NO_THROW(Worker::addWork([]() {})->get());
	BOOST_CHECK_NO_THROW(Worker::addWork([](int) {}, 0)->get());
	BOOST_CHECK_NO_THROW(Worker::addWork([](int, int) {}, 0, 0)->get());
}

BOOST_AUTO_TEST_CASE(lambda_value)
{
	BOOST_CHECK_EQUAL(1, Worker::addWork([]() {
		return 1;
	})->get());
	BOOST_CHECK_EQUAL(2,
			Worker::addWork(
					[](int i) {
						return i;
					},
					2)
					->get());
	BOOST_CHECK_EQUAL(3,
			Worker::addWork(
					[](int i, int j) {
						return i + j;
					},
					1, 2)
					->get());
}

BOOST_AUTO_TEST_CASE(recursive, *boost::unit_test::timeout(5))
{
	auto recurse = []() {
		std::vector<Worker::WorkPtrT<uint32_t>> ps;
		for (int i {}; i < 30; ++i) {
			ps.push_back(Worker::addWork(workCounter));
		}
		return std::accumulate(ps.begin(), ps.end(), 0U, [](auto && out, auto && p) {
			return out += p->get();
		});
	};
	std::vector<Worker::WorkPtrT<uint32_t>> ps;
	for (int i {}; i < 30; ++i) {
		ps.push_back(Worker::addWork(recurse));
	}
	std::set<uint32_t> out;
	std::transform(ps.begin(), ps.end(), std::inserter(out, out.end()), [](auto && p) {
		return p->get();
	});
}
