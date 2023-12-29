#include "rail.h"
#include "network.h"
#include <array>
#include <cmath>
#include <collection.h>
#include <cstddef>
#include <game/network/link.h>
#include <game/network/network.impl.h> // IWYU pragma: keep
#include <gfx/models/vertex.h>
#include <glad/gl.h>
#include <glm/gtx/transform.hpp>
#include <initializer_list>
#include <maths.h>
#include <stdexcept>
#include <utility>
#include <vector>

template class NetworkOf<RailLink>;

constexpr auto RAIL_CROSSSECTION_VERTICES {5U};
constexpr Size3D RAIL_HEIGHT {0, 0, 250.F};

RailLinks::RailLinks() : NetworkOf<RailLink> {"rails.jpg"} { }

void
RailLinks::tick(TickDuration)
{
}

std::shared_ptr<RailLink>
RailLinks::addLinksBetween(Position3D start, Position3D end)
{
	auto node1ins = newNodeAt(start), node2ins = newNodeAt(end);
	if (node1ins.second == NodeIs::NotInNetwork && node2ins.second == NodeIs::NotInNetwork) {
		// Both nodes are new, direct link, easy
		return addLink<RailLinkStraight>(start, end);
	}
	if (node1ins.second == NodeIs::NotInNetwork && node2ins.second == NodeIs::InNetwork) {
		// node1 is new, node2 exists, but we build from existing outwards
		std::swap(node1ins, node2ins);
		std::swap(start, end);
	}
	// Find start link/end - opposite entry dir to existing link; so pi +...
	const float dir = pi + findNodeDirection(node1ins.first);
	if (dir == vector_yaw(end - start)) {
		return addLink<RailLinkStraight>(start, end);
	}
	const Position2D flatStart {start.xy()}, flatEnd {end.xy()};
	if (node2ins.second == NodeIs::InNetwork) {
		auto midheight = [&](auto mid) {
			const auto sm = glm::distance(flatStart, mid), em = glm::distance(flatEnd, mid);
			return start.z + ((end.z - start.z) * (sm / (sm + em)));
		};
		const float dir2 = pi + findNodeDirection(node2ins.first);
		if (const auto radii = find_arcs_radius(flatStart, dir, flatEnd, dir2); radii.first < radii.second) {
			const auto radius {radii.first};
			const auto c1 = flatStart + sincosf(dir + half_pi) * radius;
			const auto c2 = flatEnd + sincosf(dir2 + half_pi) * radius;
			const auto mid = (c1 + c2) / 2.F;
			const auto midh = mid || midheight(mid);
			addLink<RailLinkCurve>(start, midh, c1);
			return addLink<RailLinkCurve>(end, midh, c2);
		}
		else {
			const auto radius {radii.second};
			const auto c1 = flatStart + sincosf(dir - half_pi) * radius;
			const auto c2 = flatEnd + sincosf(dir2 - half_pi) * radius;
			const auto mid = (c1 + c2) / 2.F;
			const auto midh = mid || midheight(mid);
			addLink<RailLinkCurve>(midh, start, c1);
			return addLink<RailLinkCurve>(midh, end, c2);
		}
	}
	const auto diff {end - start};
	const auto vy {vector_yaw(diff)};
	const auto n2ed {(vy * 2) - dir - pi};
	const auto centre {find_arc_centre(flatStart, dir, flatEnd, n2ed)};

	if (centre.second) { // right hand arc
		std::swap(start, end);
	}
	return addLink<RailLinkCurve>(start, end, centre.first);
}

Mesh::Ptr
RailLink::defaultMesh(const std::span<Vertex> vertices)
{
	std::vector<unsigned int> indices;
	for (auto n = RAIL_CROSSSECTION_VERTICES; n < vertices.size(); n += 1) {
		indices.push_back(n - RAIL_CROSSSECTION_VERTICES);
		indices.push_back(n);
	}

	return std::make_unique<Mesh>(vertices, indices, GL_TRIANGLE_STRIP);
}

void
RailLink::render(const SceneShader &) const
{
	mesh->Draw();
}

constexpr const std::array<std::pair<Position3D, float>, RAIL_CROSSSECTION_VERTICES> railCrossSection {{
		//   ___________
		// _/           \_
		//  left to right
		{{-1900.F, 0.F, 0.F}, 0.F},
		{{-608.F, 0.F, RAIL_HEIGHT.z}, .34F},
		{{0, 0.F, RAIL_HEIGHT.z * .7F}, .5F},
		{{608.F, 0.F, RAIL_HEIGHT.z}, .66F},
		{{1900.F, 0.F, 0.F}, 1.F},
}};
constexpr auto sleepers {5.F}; // There are 5 repetitions of sleepers in the texture

inline auto
round_sleepers(const float v)
{
	return round_frac(v, sleepers);
}

RailLinkStraight::RailLinkStraight(const Node::Ptr & a, const Node::Ptr & b) : RailLinkStraight(a, b, b->pos - a->pos)
{
}

RailLinkStraight::RailLinkStraight(Node::Ptr a, Node::Ptr b, const Position3D & diff) :
	Link({std::move(a), vector_yaw(diff)}, {std::move(b), vector_yaw(-diff)}, glm::length(diff))
{
	if (glGenVertexArrays) {
		std::vector<Vertex> vertices;
		vertices.reserve(2 * railCrossSection.size());
		const auto len = round_sleepers(length / 2000.F);
		const auto e {flat_orientation(diff)};
		for (auto ei : {1U, 0U}) {
			const auto trans {glm::translate(ends[ei].node->pos) * e};
			for (const auto & rcs : railCrossSection) {
				const Position3D m {(trans * (rcs.first || 1.F))};
				vertices.emplace_back(m, Position2D {rcs.second, len * static_cast<float>(ei)}, up);
			}
		}
		mesh = defaultMesh(vertices);
	}
}

RailLinkCurve::RailLinkCurve(const Node::Ptr & a, const Node::Ptr & b, Position2D c) :
	RailLinkCurve(a, b, c || a->pos.z, {c || 0.F, a->pos, b->pos})
{
}

RailLinkCurve::RailLinkCurve(const Node::Ptr & a, const Node::Ptr & b, Position3D c, const Arc arc) :
	Link({a, normalize(arc.first + half_pi)}, {b, normalize(arc.second - half_pi)},
			(glm::length(a->pos - c)) * arc_length(arc)),
	LinkCurve {c, glm::length(ends[0].node->pos - c), arc}
{
	if (glGenVertexArrays) {
		const auto & e0p {ends[0].node->pos};
		const auto & e1p {ends[1].node->pos};
		const auto slength = round_sleepers(length / 2.F);
		const auto segs = std::round(slength / std::pow(radius, 0.7F));
		const auto step {Position3D {arc_length(arc), e1p.z - e0p.z, slength / 1000.F} / segs};
		const auto trans {glm::translate(centreBase)};

		auto segCount = static_cast<std::size_t>(std::lround(segs)) + 1;
		std::vector<Vertex> vertices;
		vertices.reserve(segCount * railCrossSection.size());
		for (Position3D swing = {arc.first, centreBase.z - e0p.z, 0.F}; segCount; swing += step, --segCount) {
			const auto t {
					trans * glm::rotate(half_pi - swing.x, up) * glm::translate(Position3D {radius, 0.F, swing.y})};
			for (const auto & rcs : railCrossSection) {
				const Position3D m {(t * (rcs.first || 1.F))};
				vertices.emplace_back(m, Position2D {rcs.second, swing.z}, up);
			}
		}
		mesh = defaultMesh(vertices);
	}
}

Position3D
RailLink::vehiclePositionOffset() const
{
	return RAIL_HEIGHT;
}
