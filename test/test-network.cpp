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

namespace {
	struct TestLinkS;

	struct TestLink : public virtual Link {
		using StraightLink = TestLinkS;
		using CurveLink = TestLinkS;
	};

	struct TestLinkS : public TestLink, public LinkStraight {
		TestLinkS(NetworkLinkHolder<TestLinkS> & network, const Node::Ptr & nodeA, const Node::Ptr & nodeB) :
			TestLinkS {network, nodeA, nodeB, (nodeA->pos - nodeB->pos)}
		{
		}

		TestLinkS(NetworkLinkHolder<TestLinkS> &, Node::Ptr nodeA, Node::Ptr nodeB, RelativePosition2D difference) :
			Link {{.node = std::move(nodeA), .dir = 0}, {.node = std::move(nodeB), .dir = pi}, glm::length(difference)}
		{
		}

		struct Vertex { };

		TestLinkS(NetworkLinkHolder<TestLinkS> &, Node::Ptr nodeA, Node::Ptr nodeB, float length) :
			Link {{.node = std::move(nodeA), .dir = 0}, {.node = std::move(nodeB), .dir = pi}, length}
		{
		}
	};

	constexpr GlobalPosition3D P000 {0, 0, 500}, P100 {10500, 0, 1000}, P200 {20100, 0, 2000}, P300 {30700, 0, 3000};
	constexpr GlobalPosition3D P110 {10300, 10400, 4000};
}

template<> NetworkLinkHolder<TestLinkS>::NetworkLinkHolder() = default;

namespace {
	struct TestNetwork : public NetworkOf<TestLink, TestLinkS> {
		TestNetwork() : NetworkOf<TestLink, TestLinkS> {RESDIR "rails.jpg"}
		{
			//       0        1        2
			// p000 <-> p100 <-> p200 <-> p300
			//   \        |       /
			//    \       5      /
			//     3      |     4
			//      \-> p110 <-/
			addLink<TestLinkS>(P000, P100, 1.F);
			addLink<TestLinkS>(P100, P200, 1.F);
			addLink<TestLinkS>(P200, P300, 1.F);
			addLink<TestLinkS>(P000, P110, 2.F);
			addLink<TestLinkS>(P200, P110, 2.F);
			addLink<TestLinkS>(P100, P110, 1.F);
		}

		void
		render(const SceneShader &, const Frustum &) const override
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

	constexpr auto VALID_NODES = std::array<GlobalPosition3D, 4>({
			P000,
			P100,
			P200,
			P300,
	});
	constexpr auto INVALID_NODES = std::array<GlobalPosition3D, 3>({
			{1000, 0, 0},
			{0, 1000, 0},
			{0, 0, 1000},
	});
}

BOOST_FIXTURE_TEST_SUITE(tn, TestNetwork)

BOOST_DATA_TEST_CASE(FindNodeAtValid, VALID_NODES, point)
{
	auto node = findNodeAt(point);
	BOOST_REQUIRE(node);
	BOOST_CHECK_EQUAL(node->pos, point);
}

BOOST_DATA_TEST_CASE(FindNodeAtInvalid, INVALID_NODES, point)
{
	BOOST_REQUIRE(!findNodeAt(point));
}

BOOST_DATA_TEST_CASE(NodeAt, VALID_NODES + INVALID_NODES, point)
{
	auto node = nodeAt(point);
	BOOST_REQUIRE(node);
	BOOST_CHECK_EQUAL(node->pos, point);
}

BOOST_DATA_TEST_CASE(NewNodeAtExisting, VALID_NODES, point)
{
	auto node = newNodeAt(point);
	BOOST_CHECK_EQUAL(node.second, Network::NodeIs::InNetwork);
	BOOST_REQUIRE(node.first);
	BOOST_CHECK_EQUAL(node.first->pos, point);
}

BOOST_DATA_TEST_CASE(NewNodeAtNew, INVALID_NODES, point)
{
	auto node = newNodeAt(point);
	BOOST_CHECK_EQUAL(node.second, Network::NodeIs::NotInNetwork);
	BOOST_REQUIRE(node.first);
	BOOST_CHECK_EQUAL(node.first->pos, point);
}

BOOST_AUTO_TEST_CASE(NetworkJoins)
{
	// Ends
	BOOST_CHECK(links[2]->ends[1].nexts.empty());
	// Join 0 <-> 1
	BOOST_REQUIRE_EQUAL(links[0]->ends[1].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[0].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[0].second, 0);
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[1].first.lock().get(), links[5].get());
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[1].second, 0);
	BOOST_REQUIRE_EQUAL(links[1]->ends[0].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[0].first.lock().get(), links[0].get());
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[0].second, 1);
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[1].first.lock().get(), links[5].get());
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[1].second, 0);
	// Join 1 <-> 2
	BOOST_REQUIRE_EQUAL(links[1]->ends[1].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[1]->ends[1].nexts[0].first.lock().get(), links[2].get());
	BOOST_CHECK_EQUAL(links[1]->ends[1].nexts[0].second, 0);
	BOOST_REQUIRE_EQUAL(links[2]->ends[0].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[2]->ends[0].nexts[0].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(links[2]->ends[0].nexts[0].second, 1);
}

BOOST_DATA_TEST_CASE(RouteToNodeNotInNetwork, INVALID_NODES, dest)
{
	const auto & start = links[0]->ends[1];
	BOOST_CHECK_THROW((void)routeFromTo(start, dest), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(RouteToNoSteps)
{
	const auto & start = links[0]->ends[1];
	auto route = this->routeFromTo(start, P100);
	BOOST_CHECK(route.empty());
}

BOOST_AUTO_TEST_CASE(RouteToUpStreamTo2)
{
	const auto & start = links[0]->ends[1];
	auto route = this->routeFromTo(start, P200);
	BOOST_REQUIRE_EQUAL(route.size(), 1);
	BOOST_CHECK_EQUAL(route[0].first.lock().get(), links[1].get());
}

BOOST_AUTO_TEST_CASE(RouteToUpStreamTo3)
{
	const auto & start = links[0]->ends[1];
	auto route = this->routeFromTo(start, P300);
	BOOST_REQUIRE_EQUAL(route.size(), 2);
	BOOST_CHECK_EQUAL(route[0].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(route[1].first.lock().get(), links[2].get());
}

BOOST_AUTO_TEST_CASE(RouteToDownStreamTo0)
{
	const auto & start = links[2]->ends[0];
	auto route = this->routeFromTo(start, P000);
	BOOST_REQUIRE_EQUAL(route.size(), 2);
	BOOST_CHECK_EQUAL(route[0].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(route[1].first.lock().get(), links[0].get());
}

BOOST_AUTO_TEST_CASE(RouteToUpStream3to300)
{
	const auto & start = links[3]->ends[1];
	auto route = this->routeFromTo(start, P300);
	BOOST_REQUIRE_EQUAL(route.size(), 2);
	BOOST_CHECK_EQUAL(route[0].first.lock().get(), links[4].get());
	BOOST_CHECK_EQUAL(route[1].first.lock().get(), links[2].get());
}

BOOST_AUTO_TEST_CASE(RouteToDownStream3to300)
{
	const auto & start = links[3]->ends[0];
	auto route = this->routeFromTo(start, P300);
	BOOST_REQUIRE_EQUAL(route.size(), 3);
	BOOST_CHECK_EQUAL(route[0].first.lock().get(), links[0].get());
	BOOST_CHECK_EQUAL(route[1].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(route[2].first.lock().get(), links[2].get());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_CASE(TestRailNetwork, RailLinks)
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
	auto link0 = addLinksBetween(P000, P100);
	BOOST_CHECK(dynamic_cast<RailLinkStraight *>(link0.get()));
	BOOST_CHECK_EQUAL(link0->length, ::distance(P000, P100));
	BOOST_CHECK_CLOSE(link0->ends[0].dir, half_pi, 0.1F);
	BOOST_CHECK_CLOSE(link0->ends[1].dir, -half_pi, 0.1F);
	BOOST_CHECK(link0->ends[0].nexts.empty());
	BOOST_CHECK(link0->ends[1].nexts.empty());

	auto link1 = addLinksBetween(P200, P100);
	BOOST_CHECK(dynamic_cast<RailLinkStraight *>(link1.get()));
	BOOST_CHECK_EQUAL(link1->length, ::distance(P200, P100));
	BOOST_CHECK_CLOSE(link1->ends[0].dir, half_pi, 0.1F);
	BOOST_CHECK_CLOSE(link1->ends[1].dir, -half_pi, 0.1F);
	BOOST_CHECK(link0->ends[0].nexts.empty());
	BOOST_CHECK_EQUAL(link0->ends[1].nexts.at(0).first.lock(), link1);
	BOOST_CHECK_EQUAL(link0->ends[1].nexts.at(0).second, 0);
	BOOST_CHECK_EQUAL(link1->ends[0].nexts.at(0).first.lock(), link0);
	BOOST_CHECK_EQUAL(link1->ends[0].nexts.at(0).second, 1);
	BOOST_CHECK(link1->ends[1].nexts.empty());

	auto link2 = addLinksBetween(P200, P300);
	BOOST_CHECK(dynamic_cast<RailLinkStraight *>(link2.get()));
	BOOST_CHECK_EQUAL(link2->length, ::distance(P200, P300));
	BOOST_CHECK_CLOSE(link2->ends[0].dir, half_pi, 0.1F);
	BOOST_CHECK_CLOSE(link2->ends[1].dir, -half_pi, 0.1F);
	BOOST_CHECK(link0->ends[0].nexts.empty());
	BOOST_CHECK_EQUAL(link1->ends[1].nexts.at(0).first.lock(), link2);
	BOOST_CHECK_EQUAL(link1->ends[1].nexts.at(0).second, 0);
	BOOST_CHECK_EQUAL(link2->ends[0].nexts.at(0).first.lock(), link1);
	BOOST_CHECK_EQUAL(link2->ends[0].nexts.at(0).second, 1);
	BOOST_CHECK(link2->ends[1].nexts.empty());

	BOOST_CHECK_IF(link3, addLinksBetween(P000, P110)) {
		BOOST_CHECK_IF(link3c, dynamic_cast<RailLinkCurve *>(link3.get())) {
			BOOST_CHECK_CLOSE(link3c->radius, 10'300.F, 0.1F);
			BOOST_CHECK_CLOSE(link3c->arc.length(), pi + half_pi, 0.5F);
			BOOST_CHECK_CLOSE(link3->length, 48'563.F, 0.1F);
			BOOST_CHECK_CLOSE(link3->ends[0].dir, -half_pi, 0.5F);
			BOOST_CHECK_CLOSE(link3->ends[1].dir, -0.0097F, 0.5F);
			BOOST_CHECK_EQUAL(link0->ends[0].nexts.at(0).first.lock(), link3);
			BOOST_CHECK_EQUAL(link0->ends[0].nexts.at(0).second, 0);
			BOOST_CHECK_EQUAL(link3->ends[0].nexts.at(0).first.lock(), link0);
			BOOST_CHECK_EQUAL(link3->ends[0].nexts.at(0).second, 0);
			BOOST_CHECK(link3->ends[1].nexts.empty());
		}
	}

	BOOST_CHECK_IF(link4, addLinksBetween(P110, P300)) {
		BOOST_CHECK_IF(link4c, dynamic_cast<RailLinkCurve *>(link4.get())) {
			BOOST_CHECK_CLOSE(link4c->radius, 6950.F, 0.1F);
			BOOST_CHECK_CLOSE(link4c->arc.length(), 4.456F, 0.1F);
			BOOST_CHECK_CLOSE(link4->length, 30'981.F, 0.1F);
			BOOST_CHECK_BETWEEN(link4->ends[0].dir, .25F, .26F);
			BOOST_CHECK_CLOSE(link4->ends[1].dir, half_pi, 0.1F);
		}
	}
}
