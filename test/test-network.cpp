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
	struct TestLinkC;

	struct TestLink : public virtual Link {
		using StraightLink = TestLinkS;
		using CurveLink = TestLinkC;

		struct Vertex { };
	};

	struct TestLinkS : public TestLink, public LinkStraight {
		TestLinkS(NetworkLinkHolder<TestLinkS> & network, const Node::Ptr & nodeA, const Node::Ptr & nodeB) :
			TestLinkS {network, nodeA, nodeB, (nodeA->pos - nodeB->pos)}
		{
		}

		TestLinkS(NetworkLinkHolder<TestLinkS> & network, Node::Ptr nodeA, Node::Ptr nodeB,
				RelativePosition2D difference) :
			TestLinkS {network, std::move(nodeA), std::move(nodeB), glm::length(difference), vector_yaw(difference)}
		{
		}

		TestLinkS(NetworkLinkHolder<TestLinkS> &, Node::Ptr nodeA, Node::Ptr nodeB, float length, Angle dirForward) :
			Link {{.node = std::move(nodeA), .dir = dirForward},
					{.node = std::move(nodeB), .dir = normalize(pi + dirForward)}, length}
		{
		}
	};

	struct TestLinkC : public TestLink, public LinkCurve {
		TestLinkC(NetworkLinkHolder<TestLinkC> & network, const Node::Ptr & nodeA, const Node::Ptr & nodeB,
				GlobalPosition2D centre) :
			TestLinkC {network, nodeA, nodeB, centre,
					{vector_yaw(difference(nodeA->pos.xy(), centre)), vector_yaw(difference(nodeB->pos.xy(), centre))}}
		{
		}

		TestLinkC(NetworkLinkHolder<TestLinkC> &, const Node::Ptr & nodeA, const Node::Ptr & nodeB,
				GlobalPosition2D centre, Arc arc) :
			Link {{.node = nodeA, .dir = normalize(arc.first + half_pi)},
					{.node = nodeB, .dir = normalize(arc.second - half_pi)},
					// Wrong, but OK for testing, just 10% longer than a straight line
					(glm::length(difference(nodeA->pos, nodeB->pos)) * 1.1F)},
			LinkCurve {centre || nodeA->pos.z, glm::length(difference(centre, nodeA->pos.xy())), arc}
		{
		}
	};

	constexpr GlobalPosition3D P000 {0, 0, 500}, P100 {10500, 0, 1000}, P200 {20100, 0, 2000}, P300 {30700, 0, 3000};
	constexpr GlobalPosition3D P110 {10300, 10400, 4000};
}

template<> NetworkLinkHolder<TestLinkS>::NetworkLinkHolder() = default;
template<> NetworkLinkHolder<TestLinkC>::NetworkLinkHolder() = default;

namespace {
	struct EmptyNetwork : public NetworkOf<TestLink, TestLinkS, TestLinkC> {
		EmptyNetwork() : NetworkOf<TestLink, TestLinkS, TestLinkC> {RESDIR "rails.jpg"} { }

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

		SnapPoints
		getSnapPoints() const override
		{
			return {};
		}
	};

	struct TestNetwork : public EmptyNetwork {
		TestNetwork()
		{
			/*
			 *   4--> p110 <---------6-------
			 * /            8                 \.
			 * 3            7                  5
			 *  \           \                 /
			 *   -> p000 <-0-> p100 <-1-> p200 <-2-> p300  */
			// 0, 1 and 2 - 3 straight links between 4 nodes
			add(nullptr, createChain(nullptr, std::array {P000, P100, P200, P300}));
			// 3 and 4 - RH biarc from p000 to p110
			add(nullptr,
					create(nullptr,
							{.fromEnd = {.position = P000, .direction = -half_pi},
									.toEnd = {.position = P110, .direction = -half_pi}}));
			// 5 and 6 - LH biarc from p200 to p110
			add(nullptr,
					create(nullptr,
							{.fromEnd = {.position = P200, .direction = half_pi},
									.toEnd = {.position = P110, .direction = half_pi}}));
			// 7 and 8 - RL S biarc from p100 to p110
			add(nullptr,
					create(nullptr,
							{.fromEnd = {.position = P100, .direction = -half_pi},
									.toEnd = {.position = P110, .direction = half_pi}}));
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
	BOOST_REQUIRE_EQUAL(links.size(), 9);
	// Ends
	BOOST_CHECK(links[2]->ends[1].nexts.empty());
	// Join 0 <-> 1
	BOOST_REQUIRE_EQUAL(links[0]->ends[1].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[0].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[0].second, 0);
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[1].first.lock().get(), links[7].get());
	BOOST_CHECK_EQUAL(links[0]->ends[1].nexts[1].second, 1);
	BOOST_REQUIRE_EQUAL(links[1]->ends[0].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[0].first.lock().get(), links[0].get());
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[0].second, 1);
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[1].first.lock().get(), links[7].get());
	BOOST_CHECK_EQUAL(links[1]->ends[0].nexts[1].second, 1);
	// Join 1 <-> 2
	BOOST_REQUIRE_EQUAL(links[1]->ends[1].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[1]->ends[1].nexts[0].first.lock().get(), links[2].get());
	BOOST_CHECK_EQUAL(links[1]->ends[1].nexts[0].second, 0);
	BOOST_CHECK_EQUAL(links[1]->ends[1].nexts[1].first.lock().get(), links[5].get());
	BOOST_CHECK_EQUAL(links[1]->ends[1].nexts[1].second, 0);
	BOOST_REQUIRE_EQUAL(links[2]->ends[0].nexts.size(), 2);
	BOOST_CHECK_EQUAL(links[2]->ends[0].nexts[0].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(links[2]->ends[0].nexts[0].second, 1);
	BOOST_CHECK_EQUAL(links[2]->ends[0].nexts[1].first.lock().get(), links[5].get());
	BOOST_CHECK_EQUAL(links[2]->ends[0].nexts[1].second, 0);
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
	BOOST_REQUIRE_EQUAL(route.size(), 3);
	BOOST_CHECK_EQUAL(route[0].first.lock().get(), links[0].get());
	BOOST_CHECK_EQUAL(route[1].first.lock().get(), links[1].get());
	BOOST_CHECK_EQUAL(route[2].first.lock().get(), links[2].get());
}

BOOST_AUTO_TEST_CASE(RouteToDownStream3to300)
{
	const auto & start = links[3]->ends[0];
	auto route = this->routeFromTo(start, P300);
	BOOST_REQUIRE_EQUAL(route.size(), 4);
	BOOST_CHECK_EQUAL(route[0].first.lock().get(), links[4].get());
	BOOST_CHECK_EQUAL(route[1].first.lock().get(), links[6].get());
	BOOST_CHECK_EQUAL(route[2].first.lock().get(), links[5].get());
	BOOST_CHECK_EQUAL(route[3].first.lock().get(), links[2].get());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(en, EmptyNetwork)

using GenCurveDefsData = std::tuple<GlobalPosition3D, GlobalPosition3D, Angle, GenCurveDef>;

BOOST_DATA_TEST_CASE(GenCurveDefs,
		boost::unit_test::data::make<GenCurveDefsData>({
				{{0, 0, 0}, {1000, 1000, 0}, 0, {{1000, 1000, 0}, {0, 0, 0}, {1000, 0}}},
				{{0, 0, 0}, {-1000, 1000, 0}, 0, {{0, 0, 0}, {-1000, 1000, 0}, {-1000, 0}}},
				{{0, 0, 0}, {0, 1000, 0}, half_pi, {{0, 0, 0}, {0, 1000, 0}, {0, 500}}},
				{{0, 0, 0}, {0, 1000, 0}, -half_pi, {{0, 1000, 0}, {0, 0, 0}, {0, 500}}},
		}),
		start, end, startDir, exp)
{
	const auto defs = genDef(start, end, startDir);
	BOOST_REQUIRE_EQUAL(defs.size(), 1);
	BOOST_REQUIRE_EQUAL(defs.front().index(), 1);
	BOOST_CHECK_EQUAL(std::get<GenCurveDef>(defs.front()), exp);
}

BOOST_AUTO_TEST_CASE(NetworkCreateStraight)
{
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = {}},
					.toEnd = {.position = {0, 1000, 0}, .direction = {}},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 1);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(straight, std::dynamic_pointer_cast<TestLinkS>(link.front())) {
		BOOST_CHECK_CLOSE(straight->length, 1000, 1);
		BOOST_CHECK_CLOSE_VECI(straight->ends.front().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE_VECI(straight->ends.back().node->pos, GlobalPosition3D(0, 1000, 0));
	}

	add(nullptr, link.front());
	BOOST_CHECK_EQUAL(links.size(), 1);
	BOOST_CHECK_EQUAL(nodes.size(), 2);
}

BOOST_AUTO_TEST_CASE(NetworkCreateExtendingStraight)
{
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = pi + quarter_pi},
					.toEnd = {.position = {1000, 1000, 0}, .direction = {}},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 1);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(straight, std::dynamic_pointer_cast<TestLinkS>(link.front())) {
		BOOST_CHECK_CLOSE(straight->length, 1414, 1);
		BOOST_CHECK_CLOSE_VECI(straight->ends.front().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE(straight->ends.front().dir, quarter_pi - pi, 1);
		BOOST_CHECK_CLOSE_VECI(straight->ends.back().node->pos, GlobalPosition3D(1000, 1000, 0));
		BOOST_CHECK_CLOSE(straight->ends.back().dir, quarter_pi, 1);
	}

	add(nullptr, link.front());
	BOOST_CHECK_EQUAL(links.size(), 1);
	BOOST_CHECK_EQUAL(nodes.size(), 2);
}

BOOST_AUTO_TEST_CASE(NetworkCreateExtendeeStraight)
{
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = {}},
					.toEnd = {.position = {1000, 1000, 0}, .direction = quarter_pi},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 1);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(straight, std::dynamic_pointer_cast<TestLinkS>(link.front())) {
		BOOST_CHECK_CLOSE(straight->length, 1414, 1);
		BOOST_CHECK_CLOSE_VECI(straight->ends.front().node->pos, GlobalPosition3D(1000, 1000, 0));
		BOOST_CHECK_CLOSE(straight->ends.front().dir, quarter_pi, 1);
		BOOST_CHECK_CLOSE_VECI(straight->ends.back().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE(straight->ends.back().dir, quarter_pi - pi, 1);
	}

	add(nullptr, link.front());
	BOOST_CHECK_EQUAL(links.size(), 1);
	BOOST_CHECK_EQUAL(nodes.size(), 2);
}

BOOST_AUTO_TEST_CASE(NetworkCreateExtendingCurve)
{
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = half_pi},
					.toEnd = {.position = {0, 1000, 0}, .direction = {}},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 1);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(curve, std::dynamic_pointer_cast<TestLinkC>(link.front())) {
		BOOST_CHECK_CLOSE(curve->length, 1100, 1);
		BOOST_CHECK_CLOSE(curve->radius, 500, 1);
		BOOST_CHECK_CLOSE_VECI(curve->centreBase, GlobalPosition3D(0, 500, 0));
		BOOST_CHECK_CLOSE_VECI(curve->ends.front().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE(curve->ends.front().dir, -half_pi, 1);
		BOOST_CHECK_CLOSE_VECI(curve->ends.back().node->pos, GlobalPosition3D(0, 1000, 0));
		BOOST_CHECK_CLOSE(curve->ends.back().dir, -half_pi, 1);
	}

	add(nullptr, link.front());
	BOOST_CHECK_EQUAL(links.size(), 1);
	BOOST_CHECK_EQUAL(nodes.size(), 2);
}

BOOST_AUTO_TEST_CASE(NetworkCreateExtendeeCurve)
{
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = {}},
					.toEnd = {.position = {0, 1000, 0}, .direction = half_pi},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 1);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(curve, std::dynamic_pointer_cast<TestLinkC>(link.front())) {
		BOOST_CHECK_CLOSE(curve->length, 1100, 1);
		BOOST_CHECK_CLOSE(curve->radius, 500, 1);
		BOOST_CHECK_CLOSE_VECI(curve->centreBase, GlobalPosition3D(0, 500, 0));
		BOOST_CHECK_CLOSE_VECI(curve->ends.front().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE(curve->ends.front().dir, -half_pi, 1);
		BOOST_CHECK_CLOSE_VECI(curve->ends.back().node->pos, GlobalPosition3D(0, 1000, 0));
		BOOST_CHECK_CLOSE(curve->ends.back().dir, -half_pi, 1);
		BOOST_CHECK_EQUAL(curve->endAt(GlobalPosition3D(0, 0, 0)), &curve->ends.front());
		BOOST_CHECK_EQUAL(curve->endAt(GlobalPosition3D(0, 1000, 0)), &curve->ends.back());
		BOOST_CHECK(!curve->endAt(GlobalPosition3D(1000, 1000, 0)));
	}

	add(nullptr, link.front());
	BOOST_CHECK_EQUAL(links.size(), 1);
	BOOST_CHECK_EQUAL(nodes.size(), 2);
}

BOOST_AUTO_TEST_CASE(NetworkCreateBiarcPair)
{
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = pi},
					.toEnd = {.position = {1000, 1000, 0}, .direction = 0},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 2);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(firstCurve, std::dynamic_pointer_cast<TestLinkC>(link.front())) {
		BOOST_CHECK_CLOSE(firstCurve->length, 777, 1);
		BOOST_CHECK_CLOSE(firstCurve->radius, 500, 1);
		BOOST_CHECK_CLOSE_VECI(firstCurve->centreBase, GlobalPosition3D(500, 0, 0));
		BOOST_CHECK_CLOSE_VECI(firstCurve->ends.front().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE(firstCurve->ends.front().dir, 0, 1);
		BOOST_CHECK_CLOSE_VECI(firstCurve->ends.back().node->pos, GlobalPosition3D(500, 500, 0));
		BOOST_CHECK_CLOSE(firstCurve->ends.back().dir, -half_pi, 1);
	}

	BOOST_CHECK_IF(secondCurve, std::dynamic_pointer_cast<TestLinkC>(link.back())) {
		BOOST_CHECK_CLOSE(secondCurve->length, 777, 1);
		BOOST_CHECK_CLOSE(secondCurve->radius, 500, 1);
		BOOST_CHECK_CLOSE_VECI(secondCurve->centreBase, GlobalPosition3D(500, 1000, 0));
		BOOST_CHECK_CLOSE_VECI(secondCurve->ends.front().node->pos, GlobalPosition3D(1000, 1000, 0));
		BOOST_CHECK_CLOSE(secondCurve->ends.front().dir, pi, 1);
		BOOST_CHECK_CLOSE_VECI(secondCurve->ends.back().node->pos, GlobalPosition3D(500, 500, 0));
		BOOST_CHECK_CLOSE(secondCurve->ends.back().dir, half_pi, 1);
	}

	add(nullptr, link.front());
	add(nullptr, link.back());
	BOOST_CHECK_EQUAL(links.size(), 2);
	BOOST_CHECK_EQUAL(nodes.size(), 3);
	BOOST_CHECK_EQUAL(link.front()->ends.back().nexts.front().first.lock(), link.back());
	BOOST_CHECK_EQUAL(link.back()->ends.back().nexts.front().first.lock(), link.front());
}

BOOST_AUTO_TEST_CASE(NetworkCreateBiarcPairEqTanLH)
{
	// Creates a biarc pair and then merges them into a single curve
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {1000, 0, 0}, .direction = 0},
					.toEnd = {.position = {0, 0, 0}, .direction = 0},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 1);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(firstCurve, std::dynamic_pointer_cast<TestLinkC>(link.front())) {
		BOOST_CHECK_CLOSE_VECI(firstCurve->ends.front().node->pos, GlobalPosition3D(1000, 0, 0));
		BOOST_CHECK_CLOSE_VECI(firstCurve->ends.back().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE_VECI(firstCurve->centreBase, GlobalPosition3D(500, 0, 0));
	}
}

BOOST_AUTO_TEST_CASE(NetworkCreateBiarcPairEqTanRH)
{
	// Creates a biarc pair and then merges them into a single curve
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = 0},
					.toEnd = {.position = {1000, 0, 0}, .direction = 0},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 1);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(firstCurve, std::dynamic_pointer_cast<TestLinkC>(link.front())) {
		BOOST_CHECK_CLOSE_VECI(firstCurve->ends.front().node->pos, GlobalPosition3D(1000, 0, 0));
		BOOST_CHECK_CLOSE_VECI(firstCurve->ends.back().node->pos, GlobalPosition3D(0, 0, 0));
		BOOST_CHECK_CLOSE_VECI(firstCurve->centreBase, GlobalPosition3D(500, 0, 0));
	}
}

BOOST_AUTO_TEST_CASE(NetworkCreateBiarcPairEqTanPerp)
{
	// This creates an equal pair of semi-circle arcs
	const auto link = create(nullptr,
			CreationDefinition {
					.fromEnd = {.position = {0, 0, 0}, .direction = 0},
					.toEnd = {.position = {1000, 0, 0}, .direction = pi},
			});
	BOOST_REQUIRE_EQUAL(link.size(), 2);
	BOOST_CHECK(links.empty());
	BOOST_CHECK(nodes.empty());

	BOOST_CHECK_IF(firstCurve, std::dynamic_pointer_cast<TestLinkC>(link.front())) {
		BOOST_CHECK_CLOSE_VECI(firstCurve->centreBase, GlobalPosition3D(250, 0, 0));
	}

	BOOST_CHECK_IF(secondCurve, std::dynamic_pointer_cast<TestLinkC>(link.back())) {
		BOOST_CHECK_CLOSE_VECI(secondCurve->centreBase, GlobalPosition3D(750, 0, 0));
	}
}

BOOST_AUTO_TEST_SUITE_END()
