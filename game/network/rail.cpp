#include "rail.h"
#include "game/network/link.h"
#include <GL/glew.h>
#include <array>
#include <cache.h>
#include <cassert>
#include <gfx/gl/shader.h>
#include <gfx/models/texture.h>
#include <gfx/models/vertex.hpp>
#include <glm/gtx/transform.hpp>
#include <initializer_list>
#include <location.hpp>
#include <maths.h>
#include <stdexcept>
#include <utility>

constexpr auto RAIL_CROSSSECTION_VERTICES {5U};
constexpr glm::vec3 RAIL_HEIGHT {0, .25F, 0};

RailLinks::RailLinks() : texture {Texture::cachedTexture.get("rails.jpg")} { }
void RailLinks::tick(TickDuration) { }

void
RailLinks::joinLinks(const LinkPtr & l) const
{
	for (const auto & ol : links.objects) {
		if (l != ol) {
			for (const auto oe : {0, 1}) {
				for (const auto te : {0, 1}) {
					if (l->ends[te].node == ol->ends[oe].node) {
						l->ends[te].nexts.emplace_back(ol, oe);
						ol->ends[oe].nexts.emplace_back(l, te);
					}
				}
			}
		}
	}
}

std::shared_ptr<RailLink>
RailLinks::addLinksBetween(glm::vec3 start, glm::vec3 end)
{
	auto node1ins = nodes.insert(std::make_shared<Node>(start));
	auto node2ins = nodes.insert(std::make_shared<Node>(end));
	if (node1ins.second && node2ins.second) {
		// Both nodes are new, direct link, easy
		return addLink<RailLinkStraight>(start, end);
	}
	if (node1ins.second && !node2ins.second) {
		// node1 is new, node2 exists, but we build from existing outwards
		std::swap(node1ins, node2ins);
		std::swap(start, end);
	}
	// Find start link/end - opposite entry dir to existing link; so pi +...
	const auto findDir = [this](const auto & n) {
		for (const auto & l : links.objects) {
			for (const auto & e : l->ends) {
				// cppcheck-suppress useStlAlgorithm
				if (e.node == n) {
					return e.dir;
				}
			}
		}
		throw std::runtime_error("Node exists but couldn't find it");
	};
	float dir = pi + findDir(*node1ins.first);
	const glm::vec2 flatStart {!start}, flatEnd {!end};
	if (!node2ins.second) {
		auto midheight = [&](auto mid) {
			const auto sm = glm::distance(flatStart, mid), em = glm::distance(flatEnd, mid);
			return start.y + ((end.y - start.y) * (sm / (sm + em)));
		};
		float dir2 = pi + findDir(*node2ins.first);
		if (const auto radii = find_arcs_radius(flatStart, dir, flatEnd, dir2); radii.first < radii.second) {
			const auto radius {radii.first};
			const auto c1 = flatStart + sincosf(dir + half_pi) * radius;
			const auto c2 = flatEnd + sincosf(dir2 + half_pi) * radius;
			const auto mid = (c1 + c2) / 2.F;
			const auto midh = mid ^ midheight(mid);
			addLink<RailLinkCurve>(start, midh, c1);
			return addLink<RailLinkCurve>(end, midh, c2);
		}
		else {
			const auto radius {radii.second};
			const auto c1 = flatStart + sincosf(dir - half_pi) * radius;
			const auto c2 = flatEnd + sincosf(dir2 - half_pi) * radius;
			const auto mid = (c1 + c2) / 2.F;
			const auto midh = mid ^ midheight(mid);
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

void
RailLinks::render(const Shader & shader) const
{
	shader.setModel(Location {}, Shader::Program::StaticPos);
	texture->Bind();
	links.apply(&RailLink::render, shader);
}

void
RailLink::defaultMesh()
{
	for (auto n = RAIL_CROSSSECTION_VERTICES; n < vertices.size(); n += 1) {
		indices.push_back(n - RAIL_CROSSSECTION_VERTICES);
		indices.push_back(n);
	}

	assert(vertices.capacity() == vertices.size());
	assert(indices.capacity() == indices.size());
	meshes.create<Mesh>(vertices, indices, GL_TRIANGLE_STRIP);
}

void
RailLink::render(const Shader &) const
{
	meshes.apply(&Mesh::Draw);
}

constexpr const std::array<std::pair<glm::vec3, float>, RAIL_CROSSSECTION_VERTICES> railCrossSection {{
		//   ___________
		// _/           \_
		//  left to right
		{{-1.9F, 0.F, 0.F}, 0.F},
		{{-.608F, RAIL_HEIGHT.y, 0.F}, 0.34F},
		{{0, RAIL_HEIGHT.y * .7F, 0.F}, 0.5F},
		{{.608F, RAIL_HEIGHT.y, 0.F}, 0.66F},
		{{1.9F, 0.F, 0.F}, 1.F},
}};
constexpr auto sleepers {5.F}; // There are 5 repetitions of sleepers in the texture

inline auto
round_sleepers(const float v)
{
	return round_frac(v, sleepers);
}

RailLinkStraight::RailLinkStraight(const NodePtr & a, const NodePtr & b) : RailLinkStraight(a, b, b->pos - a->pos) { }

RailLinkStraight::RailLinkStraight(NodePtr a, NodePtr b, const glm::vec3 & diff) :
	RailLink({std::move(a), vector_yaw(diff)}, {std::move(b), vector_yaw(-diff)}, glm::length(diff))
{
	vertices.reserve(2 * railCrossSection.size());
	indices.reserve(2 * railCrossSection.size());
	const auto len = round_sleepers(length / 2.F);
	const auto e {flat_orientation(diff)};
	for (int ei = 0; ei < 2; ei++) {
		const auto trans {glm::translate(ends[ei].node->pos) * e};
		for (const auto & rcs : railCrossSection) {
			const glm::vec3 m {(trans * glm::vec4 {rcs.first, 1})};
			vertices.emplace_back(m, glm::vec2 {rcs.second, ei ? len : 0.F}, up);
		}
	}
	defaultMesh();
}

Location
RailLinkStraight::positionAt(float dist, unsigned char start) const
{
	const auto es {std::make_pair(ends[start].node.get(), ends[1 - start].node.get())};
	const auto diff {es.second->pos - es.first->pos};
	const auto dir {glm::normalize(diff)};
	return Location {es.first->pos + RAIL_HEIGHT + dir * dist, {-vector_pitch(dir), vector_yaw(dir), 0}};
}

RailLinkCurve::RailLinkCurve(const NodePtr & a, const NodePtr & b, glm::vec2 c) :
	RailLinkCurve(a, b, {c.x, a->pos.y, c.y}, {!c, a->pos, b->pos})
{
}

RailLinkCurve::RailLinkCurve(const NodePtr & a, const NodePtr & b, glm::vec3 c, const Arc arc) :
	RailLink({a, normalize(arc.first + half_pi)}, {b, normalize(arc.second - half_pi)},
			(glm::length(a->pos - c)) * arc_length(arc)),
	centreBase(c), radius {glm::length(ends[0].node->pos - centreBase)}, arc {arc}
{
	const auto & e0p {ends[0].node->pos};
	const auto & e1p {ends[1].node->pos};
	const auto slength = round_sleepers(length / 2.F);
	const auto segs = std::round(5.F * slength / std::pow(radius, 0.7F));
	const auto step {glm::vec3 {-arc_length(arc), e0p.y - e1p.y, slength} / segs};
	const auto trans {glm::translate(centreBase)};

	int segCount = segs;
	vertices.reserve((segCount + 1) * railCrossSection.size());
	indices.reserve(segCount * 2 * railCrossSection.size());
	for (glm::vec3 swing = {arc.second, e1p.y - centreBase.y, 0.F}; segCount >= 0; swing += step, --segCount) {
		const auto t {trans * glm::rotate(swing.x - half_pi, up) * glm::translate(glm::vec3 {radius, swing.y, 0.F})};
		for (const auto & rcs : railCrossSection) {
			const glm::vec3 m {(t * glm::vec4 {rcs.first, 1})};
			vertices.emplace_back(m, glm::vec2 {rcs.second, swing.z}, up);
		}
	}
	defaultMesh();
}

Location
RailLinkCurve::positionAt(float dist, unsigned char start) const
{
	static constexpr std::array<float, 2> dirOffset {half_pi, -half_pi};
	const auto frac {dist / length};
	const auto es {std::make_pair(ends[start].node.get(), ends[1 - start].node.get())};
	const auto as {std::make_pair(arc[start], arc[1 - start])};
	const auto ang {as.first + ((as.second - as.first) * frac)};
	const auto relPos {!sincosf(ang) * radius};
	const auto relClimb {RAIL_HEIGHT
			+ glm::vec3 {0, -centreBase.y + es.first->pos.y + ((es.second->pos.y - es.first->pos.y) * frac), 0}};
	const auto pitch {vector_pitch({0, (es.first->pos.y - es.second->pos.y) / length, 0})};
	return Location {relPos + relClimb + centreBase, {pitch, normalize(ang + dirOffset[start]), 0}};
}
