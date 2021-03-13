#define BOOST_TEST_MODULE network

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <array>
#include <collection.hpp>
#include <game/network/link.h>
#include <game/network/network.h>
#include <game/network/network.impl.h> // IWYU pragma: keep
#include <glm/glm.hpp>
#include <maths.h>
#include <memory>
#include <stdexcept>
#include <stream_support.hpp>
#include <utility>
#include <vector>

struct TestLink : public LinkStraight {
	TestLink(NodePtr a, NodePtr b, float l) : Link {{std::move(a), 0}, {std::move(b), pi}, l} { }
};

constexpr glm::vec3 p000 {0, 0, 0}, p100 {1, 0, 0}, p200 {2, 0, 0}, p300 {3, 0, 0};
constexpr glm::vec3 p101 {1, 0, 1};

struct TestNetwork : public NetworkOf<TestLink> {
	TestNetwork() : NetworkOf<TestLink> {RESDIR "rails.jpg"}
	{
		//       0        1        2
		// p000 <-> p100 <-> p200 <-> p300
		//   \        |       /
		//    \       5      /
		//     3      |     4
		//      \-> p101 <-/
		addLink<TestLink>(p000, p100, 1.F);
		addLink<TestLink>(p100, p200, 1.F);
		addLink<TestLink>(p200, p300, 1.F);
		addLink<TestLink>(p000, p101, 2.F);
		addLink<TestLink>(p200, p101, 2.F);
		addLink<TestLink>(p100, p101, 1.F);
	}
};

const auto VALID_NODES = boost::unit_test::data::make<glm::vec3>({
		p000,
		p100,
		p200,
		p300,
});
const auto INVALID_NODES = boost::unit_test::data::make<glm::vec3>({
		{1000, 0, 0},
		{0, 1000, 0},
		{0, 0, 1000},
});

BOOST_FIXTURE_TEST_SUITE(tn, TestNetwork)

BOOST_DATA_TEST_CASE(findNodeAt_valid, VALID_NODES, p)
{
	auto n = findNodeAt(p);
	BOOST_REQUIRE(n);
	BOOST_CHECK_EQUAL(n->pos, p);
}

BOOST_DATA_TEST_CASE(findNodeAt_invalid, INVALID_NODES, p)
{
	BOOST_REQUIRE(!findNodeAt(p));
}

BOOST_DATA_TEST_CASE(nodeAt, VALID_NODES + INVALID_NODES, p)
{
	auto n = nodeAt(p);
	BOOST_REQUIRE(n);
	BOOST_CHECK_EQUAL(n->pos, p);
}

BOOST_DATA_TEST_CASE(newNodeAt_existing, VALID_NODES, p)
{
	auto n = newNodeAt(p);
	BOOST_CHECK(!n.second);
	BOOST_REQUIRE(n.first);
	BOOST_CHECK_EQUAL(n.first->pos, p);
}

BOOST_DATA_TEST_CASE(newNodeAt_new, INVALID_NODES, p)
{
	auto n = newNodeAt(p);
	BOOST_CHECK(n.second);
	BOOST_REQUIRE(n.first);
	BOOST_CHECK_EQUAL(n.first->pos, p);
}

BOOST_AUTO_TEST_CASE(network_joins)
{
	// Ends
	BOOST_CHECK(links.objects[2]->ends[1].nexts.empty());
	// Join 0 <-> 1
	BOOST_REQUIRE_EQUAL(links.objects[0]->ends[1].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links.objects[0]->ends[1].nexts[0].first.lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(links.objects[0]->ends[1].nexts[0].second, 0);
	BOOST_CHECK_EQUAL(links.objects[0]->ends[1].nexts[1].first.lock().get(), links.objects[5].get());
	BOOST_CHECK_EQUAL(links.objects[0]->ends[1].nexts[1].second, 0);
	BOOST_REQUIRE_EQUAL(links.objects[1]->ends[0].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links.objects[1]->ends[0].nexts[0].first.lock().get(), links.objects[0].get());
	BOOST_CHECK_EQUAL(links.objects[1]->ends[0].nexts[0].second, 1);
	BOOST_CHECK_EQUAL(links.objects[1]->ends[0].nexts[1].first.lock().get(), links.objects[5].get());
	BOOST_CHECK_EQUAL(links.objects[1]->ends[0].nexts[1].second, 0);
	// Join 1 <-> 2
	BOOST_REQUIRE_EQUAL(links.objects[1]->ends[1].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links.objects[1]->ends[1].nexts[0].first.lock().get(), links.objects[2].get());
	BOOST_CHECK_EQUAL(links.objects[1]->ends[1].nexts[0].second, 0);
	BOOST_REQUIRE_EQUAL(links.objects[2]->ends[0].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links.objects[2]->ends[0].nexts[0].first.lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(links.objects[2]->ends[0].nexts[0].second, 1);
}

BOOST_DATA_TEST_CASE(routeTo_nodeNotInNetwork, INVALID_NODES, dest)
{
	const auto & start = links.objects.front()->ends[1];
	BOOST_CHECK_THROW((void)routeFromTo(start, dest), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(routeTo_noSteps)
{
	const auto & start = links.objects.front()->ends[1];
	auto r = this->routeFromTo(start, p100);
	BOOST_CHECK(r.empty());
}

BOOST_AUTO_TEST_CASE(routeTo_upStream_to2)
{
	const auto & start = links.objects.front()->ends[1];
	auto r = this->routeFromTo(start, p200);
	BOOST_REQUIRE_EQUAL(r.size(), 1);
	BOOST_CHECK_EQUAL(r[0].lock().get(), links.objects[1].get());
}

BOOST_AUTO_TEST_CASE(routeTo_upStream_to3)
{
	const auto & start = links.objects[0]->ends[1];
	auto r = this->routeFromTo(start, p300);
	BOOST_REQUIRE_EQUAL(r.size(), 2);
	BOOST_CHECK_EQUAL(r[0].lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(r[1].lock().get(), links.objects[2].get());
}

BOOST_AUTO_TEST_CASE(routeTo_downStream_to0)
{
	const auto & start = links.objects[2]->ends[0];
	auto r = this->routeFromTo(start, p000);
	BOOST_REQUIRE_EQUAL(r.size(), 2);
	BOOST_CHECK_EQUAL(r[0].lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(r[1].lock().get(), links.objects[0].get());
}

BOOST_AUTO_TEST_CASE(routeTo_upStream_3to300)
{
	const auto & start = links.objects[3]->ends[1];
	auto r = this->routeFromTo(start, p300);
	BOOST_REQUIRE_EQUAL(r.size(), 2);
	BOOST_CHECK_EQUAL(r[0].lock().get(), links.objects[4].get());
	BOOST_CHECK_EQUAL(r[1].lock().get(), links.objects[2].get());
}

BOOST_AUTO_TEST_CASE(routeTo_downStream_3to300)
{
	const auto & start = links.objects[3]->ends[0];
	auto r = this->routeFromTo(start, p300);
	BOOST_REQUIRE_EQUAL(r.size(), 3);
	BOOST_CHECK_EQUAL(r[0].lock().get(), links.objects[0].get());
	BOOST_CHECK_EQUAL(r[1].lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(r[2].lock().get(), links.objects[2].get());
}

BOOST_AUTO_TEST_SUITE_END()
