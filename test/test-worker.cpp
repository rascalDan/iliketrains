#define BOOST_TEST_MODULE test_worker

#include <boost/test/unit_test.hpp>
#include <set>
#include <stream_support.h>
#include <worker.h>

namespace {
	uint32_t
	workCounter()
	{
		static std::atomic_uint32_t counter;
		usleep(1000);
		return counter++;
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
}

BOOST_AUTO_TEST_CASE(BasicSlowCounter)
{
	std::vector<Worker::WorkPtrT<uint32_t>> jobs;
	for (int i {}; i < 30; ++i) {
		jobs.emplace_back(Worker::addWork(workCounter));
	}
	std::set<uint32_t> out;
	std::ranges::transform(jobs, std::inserter(out, out.end()), &Worker::WorkItemT<uint32_t>::get);
	BOOST_REQUIRE_EQUAL(out.size(), jobs.size());
	BOOST_CHECK_EQUAL(*out.begin(), 0);
	BOOST_CHECK_EQUAL(*out.rbegin(), jobs.size() - 1);
}

BOOST_AUTO_TEST_CASE(BasicErrorHandler)
{
	auto workitem = Worker::addWork(workFail);
	BOOST_CHECK_THROW(workitem->get(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(BasicVoidWork)
{
	auto workitem = Worker::addWork(workVoid);
	BOOST_CHECK_NO_THROW(workitem->get());
}

BOOST_AUTO_TEST_CASE(LambdaVoid)
{
	BOOST_CHECK_NO_THROW(Worker::addWork([]() { })->get());
	BOOST_CHECK_NO_THROW(Worker::addWork([](int) { }, 0)->get());
	BOOST_CHECK_NO_THROW(Worker::addWork([](int, int) { }, 0, 0)->get());
}

BOOST_AUTO_TEST_CASE(LambdaValue)
{
	BOOST_CHECK_EQUAL(1, Worker::addWork([]() {
		return 1;
	})->get());
	BOOST_CHECK_EQUAL(2,
			Worker::addWork(
					[](int value) {
						return value;
					},
					2)
					->get());
	BOOST_CHECK_EQUAL(3,
			Worker::addWork(
					[](int valueA, int valueB) {
						return valueA + valueB;
					},
					1, 2)
					->get());
}

BOOST_AUTO_TEST_CASE(Recursive, *boost::unit_test::timeout(5))
{
	auto recurse = []() {
		std::vector<Worker::WorkPtrT<uint32_t>> jobs;
		for (int i {}; i < 30; ++i) {
			jobs.emplace_back(Worker::addWork(workCounter));
		}
		return std::ranges::fold_left(jobs, 0U, [](auto && out, auto && job) {
			return out += job->get();
		});
	};
	std::vector<Worker::WorkPtrT<uint32_t>> jobs;
	for (int i {}; i < 30; ++i) {
		jobs.emplace_back(Worker::addWork(recurse));
	}
	std::set<uint32_t> out;
	std::ranges::transform(jobs, std::inserter(out, out.end()), &Worker::WorkItemT<uint32_t>::get);
}
