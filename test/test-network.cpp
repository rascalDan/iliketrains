#define BOOST_TEST_MODULE network

#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "testMainWindow.h"
#include <array>
#include <collection.h>
#include <game/network/link.h>
#include <game/network/network.h>
#include <game/network/network.impl.h> // IWYU pragma: keep
#include <game/network/rail.h>
#include <glm/glm.hpp>
#include <maths.h>
#include <memory>
#include <stdexcept>
#include <stream_support.h>
#include <utility>
#include <vector>

BOOST_GLOBAL_FIXTURE(TestMainWindowAppBase);

struct TestLinkS;

struct TestLink : public virtual Link {
	using StraightLink = TestLinkS;
	using CurveLink = TestLinkS;
};

struct TestLinkS : public TestLink, public LinkStraight {
	TestLinkS(NetworkLinkHolder<TestLinkS> & network, const Node::Ptr & a, const Node::Ptr & b) :
		TestLinkS {network, a, b, (a->pos - b->pos)}
	{
	}

	TestLinkS(NetworkLinkHolder<TestLinkS> &, Node::Ptr a, Node::Ptr b, RelativePosition2D l) :
		Link {{std::move(a), 0}, {std::move(b), pi}, glm::length(l)}
	{
	}

	struct Vertex { };

	TestLinkS(NetworkLinkHolder<TestLinkS> &, Node::Ptr a, Node::Ptr b, float l) :
		Link {{std::move(a), 0}, {std::move(b), pi}, l}
	{
	}
};

constexpr GlobalPosition3D p000 {0, 0, 500}, p100 {10500, 0, 1000}, p200 {20100, 0, 2000}, p300 {30700, 0, 3000};
constexpr GlobalPosition3D p110 {10300, 10400, 4000};

template<> NetworkLinkHolder<TestLinkS>::NetworkLinkHolder() = default;

struct TestNetwork : public NetworkOf<TestLink, TestLinkS> {
	TestNetwork() : NetworkOf<TestLink, TestLinkS> {RESDIR "rails.jpg"}
	{
		//       0        1        2
		// p000 <-> p100 <-> p200 <-> p300
		//   \        |       /
		//    \       5      /
		//     3      |     4
		//      \-> p110 <-/
		addLink<TestLinkS>(p000, p100, 1.F);
		addLink<TestLinkS>(p100, p200, 1.F);
		addLink<TestLinkS>(p200, p300, 1.F);
		addLink<TestLinkS>(p000, p110, 2.F);
		addLink<TestLinkS>(p200, p110, 2.F);
		addLink<TestLinkS>(p100, p110, 1.F);
	}

	void
	render(const SceneShader &) const override
	{
	}

	const Surface *
	getBaseSurface() const override
	{
		return nullptr;
	}

	RelativeDistance
	getBaseWidth() const override
	{
		return 5'700;
	}
};

const auto VALID_NODES = boost::unit_test::data::make<GlobalPosition3D>({
		p000,
		p100,
		p200,
		p300,
});
const auto INVALID_NODES = boost::unit_test::data::make<GlobalPosition3D>({
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
	BOOST_CHECK_EQUAL(n.second, Network::NodeIs::InNetwork);
	BOOST_REQUIRE(n.first);
	BOOST_CHECK_EQUAL(n.first->pos, p);
}

BOOST_DATA_TEST_CASE(newNodeAt_new, INVALID_NODES, p)
{
	auto n = newNodeAt(p);
	BOOST_CHECK_EQUAL(n.second, Network::NodeIs::NotInNetwork);
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
	BOOST_CHECK_EQUAL(r[0].first.lock().get(), links.objects[1].get());
}

BOOST_AUTO_TEST_CASE(routeTo_upStream_to3)
{
	const auto & start = links.objects[0]->ends[1];
	auto r = this->routeFromTo(start, p300);
	BOOST_REQUIRE_EQUAL(r.size(), 2);
	BOOST_CHECK_EQUAL(r[0].first.lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(r[1].first.lock().get(), links.objects[2].get());
}

BOOST_AUTO_TEST_CASE(routeTo_downStream_to0)
{
	const auto & start = links.objects[2]->ends[0];
	auto r = this->routeFromTo(start, p000);
	BOOST_REQUIRE_EQUAL(r.size(), 2);
	BOOST_CHECK_EQUAL(r[0].first.lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(r[1].first.lock().get(), links.objects[0].get());
}

BOOST_AUTO_TEST_CASE(routeTo_upStream_3to300)
{
	const auto & start = links.objects[3]->ends[1];
	auto r = this->routeFromTo(start, p300);
	BOOST_REQUIRE_EQUAL(r.size(), 2);
	BOOST_CHECK_EQUAL(r[0].first.lock().get(), links.objects[4].get());
	BOOST_CHECK_EQUAL(r[1].first.lock().get(), links.objects[2].get());
}

BOOST_AUTO_TEST_CASE(routeTo_downStream_3to300)
{
	const auto & start = links.objects[3]->ends[0];
	auto r = this->routeFromTo(start, p300);
	BOOST_REQUIRE_EQUAL(r.size(), 3);
	BOOST_CHECK_EQUAL(r[0].first.lock().get(), links.objects[0].get());
	BOOST_CHECK_EQUAL(r[1].first.lock().get(), links.objects[1].get());
	BOOST_CHECK_EQUAL(r[2].first.lock().get(), links.objects[2].get());
}

BOOST_AUTO_TEST_SUITE_END()

namespace std {
	std::ostream &
	operator<<(std::ostream & s, const Link::End & e)
	{
		return s << std::format("End[dir: {}, loc: ({}, {}, {})]", e.dir, e.node->pos.x, e.node->pos.y, e.node->pos.z);
	}
}

BOOST_FIXTURE_TEST_CASE(test_rail_network, RailLinks)
{
	//       0        1        2
	//   --p000 <-> p100 <-> p200 <-> p300 \         x
	//  /              ?-----               \        x
	// /              /       \             |
	// |             /         4            /
	// 3          p110          \          /
	//  \          |             \        /
	//   \        /                ------/
	//    --------
	auto l0 = addLinksBetween(p000, p100);
	BOOST_CHECK(dynamic_cast<RailLinkStraight *>(l0.get()));
	BOOST_CHECK_EQUAL(l0->length, ::distance(p000, p100));
	BOOST_CHECK_CLOSE(l0->ends[0].dir, half_pi, 0.1F);
	BOOST_CHECK_CLOSE(l0->ends[1].dir, -half_pi, 0.1F);
	BOOST_CHECK(l0->ends[0].nexts.empty());
	BOOST_CHECK(l0->ends[1].nexts.empty());

	auto l1 = addLinksBetween(p200, p100);
	BOOST_CHECK(dynamic_cast<RailLinkStraight *>(l1.get()));
	BOOST_CHECK_EQUAL(l1->length, ::distance(p200, p100));
	BOOST_CHECK_CLOSE(l1->ends[0].dir, half_pi, 0.1F);
	BOOST_CHECK_CLOSE(l1->ends[1].dir, -half_pi, 0.1F);
	BOOST_CHECK(l0->ends[0].nexts.empty());
	BOOST_CHECK_EQUAL(l0->ends[1].nexts.at(0).first.lock(), l1);
	BOOST_CHECK_EQUAL(l0->ends[1].nexts.at(0).second, 0);
	BOOST_CHECK_EQUAL(l1->ends[0].nexts.at(0).first.lock(), l0);
	BOOST_CHECK_EQUAL(l1->ends[0].nexts.at(0).second, 1);
	BOOST_CHECK(l1->ends[1].nexts.empty());

	auto l2 = addLinksBetween(p200, p300);
	BOOST_CHECK(dynamic_cast<RailLinkStraight *>(l2.get()));
	BOOST_CHECK_EQUAL(l2->length, ::distance(p200, p300));
	BOOST_CHECK_CLOSE(l2->ends[0].dir, half_pi, 0.1F);
	BOOST_CHECK_CLOSE(l2->ends[1].dir, -half_pi, 0.1F);
	BOOST_CHECK(l0->ends[0].nexts.empty());
	BOOST_CHECK_EQUAL(l1->ends[1].nexts.at(0).first.lock(), l2);
	BOOST_CHECK_EQUAL(l1->ends[1].nexts.at(0).second, 0);
	BOOST_CHECK_EQUAL(l2->ends[0].nexts.at(0).first.lock(), l1);
	BOOST_CHECK_EQUAL(l2->ends[0].nexts.at(0).second, 1);
	BOOST_CHECK(l2->ends[1].nexts.empty());

	BOOST_CHECK_IF(l3, addLinksBetween(p000, p110)) {
		BOOST_CHECK_IF(l3c, dynamic_cast<RailLinkCurve *>(l3.get())) {
			BOOST_CHECK_CLOSE(l3c->radius, 10'300.F, 0.1F);
			BOOST_CHECK_CLOSE(l3c->arc.length(), pi + half_pi, 0.5F);
			BOOST_CHECK_CLOSE(l3->length, 48'563.F, 0.1F);
			BOOST_CHECK_CLOSE(l3->ends[0].dir, -half_pi, 0.5F);
			BOOST_CHECK_CLOSE(l3->ends[1].dir, -0.0097F, 0.5F);
			BOOST_CHECK_EQUAL(l0->ends[0].nexts.at(0).first.lock(), l3);
			BOOST_CHECK_EQUAL(l0->ends[0].nexts.at(0).second, 0);
			BOOST_CHECK_EQUAL(l3->ends[0].nexts.at(0).first.lock(), l0);
			BOOST_CHECK_EQUAL(l3->ends[0].nexts.at(0).second, 0);
			BOOST_CHECK(l3->ends[1].nexts.empty());
		}
	}

	BOOST_CHECK_IF(l4, addLinksBetween(p110, p300)) {
		BOOST_CHECK_IF(l4c, dynamic_cast<RailLinkCurve *>(l4.get())) {
			BOOST_CHECK_CLOSE(l4c->radius, 6950.F, 0.1F);
			BOOST_CHECK_CLOSE(l4c->arc.length(), 4.456F, 0.1F);
			BOOST_CHECK_CLOSE(l4->length, 30'981.F, 0.1F);
			BOOST_CHECK_BETWEEN(l4->ends[0].dir, .25F, .26F);
			BOOST_CHECK_CLOSE(l4->ends[1].dir, half_pi, 0.1F);
		}
	}
}
