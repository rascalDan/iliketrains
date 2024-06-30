#define BOOST_TEST_MODULE instancing

#include "stream_support.h"
#include "testHelpers.h"
#include "testMainWindow.h"
#include "ui/applicationBase.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <set>

#include <gfx/gl/instanceVertices.h>

BOOST_GLOBAL_FIXTURE(ApplicationBase);
BOOST_GLOBAL_FIXTURE(TestMainWindow);

struct TestInstanceVertices : public InstanceVertices<int> {
	void
	checkReverseIndex(std::source_location ctx = std::source_location::current())
	{
		BOOST_TEST_CONTEXT(ctx.function_name() << ":" << ctx.line()) {
			std::vector<size_t> genIndexIndex(size(), npos);
			for (size_t i {}; i < index.size(); i++) {
				if (index[i] != npos) {
					BOOST_REQUIRE_EQUAL(genIndexIndex.at(index[i]), npos);
					genIndexIndex.at(index[i]) = i;
				}
			}

			BOOST_TEST_CONTEXT(reverseIndex << genIndexIndex) {
				BOOST_CHECK_EQUAL_COLCOL(reverseIndex, genIndexIndex);
			}
		}
	}
};

BOOST_FIXTURE_TEST_SUITE(i, TestInstanceVertices)

BOOST_AUTO_TEST_CASE(createDestroy)
{
	BOOST_CHECK(unused.empty());
	BOOST_CHECK(index.empty());
	BOOST_CHECK(reverseIndex.empty());
}

BOOST_AUTO_TEST_CASE(acquireRelease)
{
	{
		auto proxy = acquire();
		*proxy = 20;
		BOOST_CHECK_EQUAL(1, size());
		BOOST_REQUIRE_EQUAL(1, index.size());
		BOOST_CHECK_EQUAL(0, index.front());
		BOOST_REQUIRE_EQUAL(1, reverseIndex.size());
		BOOST_CHECK_EQUAL(0, reverseIndex.front());
		BOOST_CHECK(unused.empty());
	}
	BOOST_CHECK_EQUAL(0, size());
	BOOST_CHECK(unused.empty());
	BOOST_CHECK(index.empty());
	BOOST_CHECK(reverseIndex.empty());
}

BOOST_AUTO_TEST_CASE(acquireReleaseMove)
{
	{
		auto proxy1 = acquire();
		*proxy1 = 20;
		BOOST_CHECK_EQUAL(1, size());
		auto proxy2 = std::move(proxy1);
		proxy2 = 40;
		BOOST_CHECK_EQUAL(1, size());
		BOOST_CHECK_EQUAL(at(0), 40);
	}
	BOOST_CHECK_EQUAL(0, size());
	BOOST_CHECK(unused.empty());
	BOOST_CHECK(index.empty());
	BOOST_CHECK(reverseIndex.empty());
}

BOOST_AUTO_TEST_CASE(autoMapUnmap)
{
	{
		auto proxy = acquire();
		BOOST_CHECK(data_);
		std::ignore = bufferName();
		BOOST_CHECK(data_);
		BOOST_CHECK_EQUAL(1, size());
		BOOST_CHECK(!data_);
	}
	BOOST_CHECK_EQUAL(0, size());
}

BOOST_AUTO_TEST_CASE(initialize)
{
	auto proxy = acquire(5);
	const auto & constProxy = proxy;
	BOOST_CHECK_EQUAL(*proxy, 5);
	BOOST_CHECK_EQUAL(*constProxy, 5);
	BOOST_CHECK_EQUAL(constProxy.get(), constProxy.get());
}

BOOST_AUTO_TEST_CASE(resize)
{
	constexpr auto COUNT = 500;
	std::vector<decltype(acquire())> proxies;
	std::vector<int> expected;
	for (auto n = 0; n < COUNT; n++) {
		proxies.push_back(acquire(n));
		expected.emplace_back(n);
	}
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), data(), data() + COUNT);
	BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), proxies.begin(), proxies.end());
}

BOOST_AUTO_TEST_CASE(shuffle)
{
	std::vector<decltype(acquire())> proxies;
	BOOST_CHECK_EQUAL(0, proxies.emplace_back(acquire(0)));
	BOOST_CHECK_EQUAL(1, proxies.emplace_back(acquire(1)));
	BOOST_CHECK_EQUAL(2, proxies.emplace_back(acquire(2)));
	BOOST_CHECK_EQUAL(3, proxies.emplace_back(acquire(3)));
	BOOST_CHECK_EQUAL(4, size());
	BOOST_CHECK_EQUAL(at(0), *proxies[0].get());
	BOOST_CHECK_EQUAL(at(1), *proxies[1].get());
	BOOST_CHECK_EQUAL(at(2), *proxies[2].get());
	BOOST_CHECK_EQUAL(at(3), *proxies[3].get());
	BOOST_CHECK(unused.empty());
	BOOST_CHECK_EQUAL_COLVALS(index, 0, 1, 2, 3);
	checkReverseIndex();
	// Remove 1, 3 moves to [1]
	proxies.erase(proxies.begin() + 1);
	BOOST_REQUIRE_EQUAL(3, proxies.size());
	BOOST_CHECK_EQUAL_COLVALS(index, 0, npos, 2, 1);
	BOOST_CHECK_EQUAL_COLVALS(reverseIndex, 0, 3, 2);
	BOOST_REQUIRE_EQUAL(1, unused.size());
	BOOST_CHECK_EQUAL(1, unused[0]);
	BOOST_CHECK_EQUAL(at(0), *proxies[0].get());
	BOOST_CHECK_EQUAL(at(2), *proxies[1].get());
	BOOST_CHECK_EQUAL(at(1), *proxies[2].get());
	//  Remove 1, 2 moves to [1]
	proxies.erase(proxies.begin() + 1);
	BOOST_REQUIRE_EQUAL(4, index.size());
	BOOST_CHECK_EQUAL_COLVALS(index, 0, npos, npos, 1);
	checkReverseIndex();
	BOOST_REQUIRE_EQUAL(2, unused.size());
	BOOST_CHECK_EQUAL(1, unused[0]);
	BOOST_CHECK_EQUAL(2, unused[1]);
	BOOST_CHECK_EQUAL(at(0), *proxies[0].get());
	BOOST_CHECK_EQUAL(at(1), *proxies[1].get());
	// Add new, takes 2 at [2]
	BOOST_CHECK_EQUAL(4, proxies.emplace_back(acquire(4)));
	BOOST_CHECK_EQUAL_COLVALS(index, 0, npos, 2, 1);
	checkReverseIndex();
	BOOST_REQUIRE_EQUAL(1, unused.size());
	BOOST_CHECK_EQUAL(1, unused[0]);
	BOOST_CHECK_EQUAL(at(0), *proxies[0].get());
	BOOST_CHECK_EQUAL(at(1), *proxies[1].get());
	BOOST_CHECK_EQUAL(at(2), *proxies[2].get());
	// Add new, takes 1 at [3]
	BOOST_CHECK_EQUAL(5, proxies.emplace_back(acquire(5)));
	BOOST_REQUIRE_EQUAL(proxies.size(), reverseIndex.size());
	BOOST_CHECK_EQUAL_COLVALS(index, 0, 3, 2, 1);
	checkReverseIndex();
	// Add new, takes 4 at [4]
	BOOST_CHECK_EQUAL(6, proxies.emplace_back(acquire(6)));
	BOOST_REQUIRE_EQUAL(proxies.size(), reverseIndex.size());
	BOOST_CHECK_EQUAL_COLVALS(index, 0, 3, 2, 1, 4);
	checkReverseIndex();
	// Remove [0]
	proxies.erase(proxies.begin());
	BOOST_REQUIRE_EQUAL(proxies.size(), reverseIndex.size());
	BOOST_CHECK_EQUAL_COLVALS(index, npos, 3, 2, 1, 0);
	checkReverseIndex();
	// Remove [3]
	proxies.erase(proxies.begin());
	BOOST_REQUIRE_EQUAL(proxies.size(), reverseIndex.size());
	BOOST_CHECK_EQUAL_COLVALS(index, npos, 1, 2, npos, 0);
	checkReverseIndex();
}

BOOST_DATA_TEST_CASE(shuffle_random, boost::unit_test::data::xrange(0, 10), x)
{
	std::ignore = x;
	std::mt19937 gen(std::random_device {}());
	std::map<int, InstanceVertices<int>::InstanceProxy> proxies;
	const std::string_view actions = "aaaaaaaarararrraarrrararararaarrrarararararararararraarrrraaaarararaararar";
	int n {};
	for (const auto action : actions) {
		switch (action) {
			case 'a':
				BOOST_REQUIRE_EQUAL(n, proxies.emplace(n, acquire(n)).first->second);
				n++;
				break;
			case 'r':
				BOOST_REQUIRE(!proxies.empty());
				auto e = std::next(proxies.begin(),
						std::uniform_int_distribution<> {0, static_cast<int>(proxies.size() - 1)}(gen));
				proxies.erase(e);
				break;
		}

		BOOST_REQUIRE_EQUAL(size(), proxies.size());
		for (const auto & [n, p] : proxies) {
			BOOST_REQUIRE_EQUAL(n, p);
		}
		std::set<size_t> iused;
		for (size_t i {}; i < index.size(); i++) {
			if (std::find(unused.begin(), unused.end(), i) == unused.end()) {
				iused.emplace(index[i]);
			}
		}
		checkReverseIndex();

		BOOST_TEST_CONTEXT(index) {
			BOOST_REQUIRE_EQUAL(iused.size(), size());
			if (!iused.empty()) {
				BOOST_REQUIRE_EQUAL(*iused.begin(), 0);
				BOOST_REQUIRE_EQUAL(*iused.rbegin(), size() - 1);
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(partition_by, *boost::unit_test::timeout(1))
{
	std::mt19937 gen(std::random_device {}());
	std::uniform_int_distribution<int> dist(0, 100000);
	static constexpr auto N = 1000;
	reserve(N);
	std::vector<decltype(acquire(0))> instances;
	instances.reserve(N);
	// At least one of each
	instances.push_back(acquire(1));
	instances.push_back(acquire(3));
	while (instances.size() < N) {
		instances.push_back(acquire(dist(gen)));
	}
	const std::vector<int> values(instances.begin(), instances.end());
	BOOST_REQUIRE_EQUAL(size(), N);

	const auto pred = [](auto x) {
		return (x % 3) == 0;
	};
	auto matchedEnd = partition(pred);
	// The underlying data is partitioned...
	BOOST_REQUIRE(std::is_partitioned(mkcspan().cbegin(), mkcspan().cend(), pred));
	// The external view of the data is unchanged...
	BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(), instances.cbegin(), instances.cend());
	// The partition point is right...
	BOOST_CHECK(!pred(*matchedEnd));
	BOOST_CHECK(pred(*--matchedEnd));
	checkReverseIndex();
}

BOOST_AUTO_TEST_SUITE_END()
