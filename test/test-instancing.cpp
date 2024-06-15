#define BOOST_TEST_MODULE instancing

#include "stream_support.h"
#include "testMainWindow.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <set>

#include <gfx/gl/instanceVertices.h>

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

BOOST_FIXTURE_TEST_SUITE(i, InstanceVertices<int>)

BOOST_AUTO_TEST_CASE(createDestroy)
{
	BOOST_CHECK(unused.empty());
	BOOST_CHECK(index.empty());
}

BOOST_AUTO_TEST_CASE(acquireRelease)
{
	{
		auto proxy = acquire();
		*proxy = 20;
		BOOST_CHECK_EQUAL(1, size());
		BOOST_REQUIRE_EQUAL(1, index.size());
		BOOST_CHECK_EQUAL(0, index.front());
		BOOST_CHECK(unused.empty());
	}
	BOOST_CHECK_EQUAL(0, size());
	BOOST_CHECK(unused.empty());
	BOOST_CHECK(index.empty());
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
	BOOST_REQUIRE_EQUAL(4, index.size());
	BOOST_CHECK_EQUAL(0, index[0]);
	BOOST_CHECK_EQUAL(1, index[1]);
	BOOST_CHECK_EQUAL(2, index[2]);
	BOOST_CHECK_EQUAL(3, index[3]);
	// Remove 1, 3 moves to [1]
	proxies.erase(proxies.begin() + 1);
	BOOST_REQUIRE_EQUAL(3, proxies.size());
	BOOST_REQUIRE_EQUAL(4, index.size());
	BOOST_REQUIRE_EQUAL(1, unused.size());
	BOOST_CHECK_EQUAL(1, unused[0]);
	BOOST_CHECK_EQUAL(at(0), *proxies[0].get());
	BOOST_CHECK_EQUAL(at(2), *proxies[1].get());
	BOOST_CHECK_EQUAL(at(1), *proxies[2].get());
	//  Remove 1, 2 moves to [1]
	proxies.erase(proxies.begin() + 1);
	BOOST_REQUIRE_EQUAL(4, index.size());
	BOOST_REQUIRE_EQUAL(2, unused.size());
	BOOST_CHECK_EQUAL(1, unused[0]);
	BOOST_CHECK_EQUAL(2, unused[1]);
	BOOST_CHECK_EQUAL(at(0), *proxies[0].get());
	BOOST_CHECK_EQUAL(at(1), *proxies[1].get());
	// Add new, takes 2 at [2]
	BOOST_CHECK_EQUAL(4, proxies.emplace_back(acquire(4)));
	BOOST_REQUIRE_EQUAL(4, index.size());
	BOOST_REQUIRE_EQUAL(1, unused.size());
	BOOST_CHECK_EQUAL(1, unused[0]);
	BOOST_CHECK_EQUAL(at(0), *proxies[0].get());
	BOOST_CHECK_EQUAL(at(1), *proxies[1].get());
	BOOST_CHECK_EQUAL(at(2), *proxies[2].get());
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

		BOOST_TEST_CONTEXT(index) {
			BOOST_REQUIRE_EQUAL(iused.size(), size());
			if (!iused.empty()) {
				BOOST_REQUIRE_EQUAL(*iused.begin(), 0);
				BOOST_REQUIRE_EQUAL(*iused.rbegin(), size() - 1);
			}
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
