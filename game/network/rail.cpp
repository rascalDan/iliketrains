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
#include <glm/gtx/vector_angle.hpp>
#include <maths.h>
#include <type_traits>
#include <utility>

RailLinks::RailLinks() : texture {Texture::cachedTexture.get("rails.jpg")} { }
void RailLinks::tick(TickDuration) { }

void
RailLinks::joinLinks(LinkPtr l) const
{
	for (const auto & ol : links.objects) {
		if (l != ol) {
			for (const auto oe : {0, 1}) {
				for (const auto te : {0, 1}) {
					if (l->ends[te].first == ol->ends[oe].first) {
						l->nexts[te].emplace_back(ol.get(), oe);
						ol->nexts[oe].emplace_back(l.get(), te);
					}
				}
			}
		}
	}
}

void
RailLinks::render(const Shader & shader) const
{
	shader.setModel(glm::identity<glm::mat4>());
	texture->Bind();
	links.apply(&RailLink::render, shader);
}

void
RailLink::defaultMesh()
{
	for (auto n = 4U; n < vertices.size(); n += 1) {
		indices.push_back(n - 4);
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

constexpr const std::array<std::pair<glm::vec3, float>, 4> railCrossSection {{
		//   ___________
		// _/           \_
		//  left to right
		{{-1.9F, 0.F, 0.F}, 0.F},
		{{-1.43F, .25F, 0.F}, 0.125F},
		{{1.43F, .25F, 0.F}, 0.875F},
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
	RailLink({std::move(a), flat_angle(diff)}, {std::move(b), flat_angle(-diff)}, glm::length(diff))
{
	vertices.reserve(2 * railCrossSection.size());
	indices.reserve(2 * railCrossSection.size());
	const auto len = round_sleepers(length / 2.F);
	const auto e {flat_orientation(diff)};
	for (int ei = 0; ei < 2; ei++) {
		const auto trans {glm::translate(ends[ei].first->pos) * e};
		for (const auto & rcs : railCrossSection) {
			const glm::vec3 m {(trans * glm::vec4 {rcs.first, 1})};
			vertices.emplace_back(m, glm::vec2 {rcs.second, ei ? len : 0.F}, up);
		}
	}
	defaultMesh();
}

Transform
RailLinkStraight::positionAt(float dist, unsigned char start) const
{
	const auto es {std::make_pair(ends[start].first.get(), ends[1 - start].first.get())};
	const auto diff {es.second->pos - es.first->pos};
	const auto dir {glm::normalize(diff)};
	return Transform {es.first->pos + dir * dist, {0, flat_angle(diff) /*, std::atan2(diff.x, -diff.z)*/, 0}};
}

RailLinkCurve::RailLinkCurve(const NodePtr & a, const NodePtr & b, glm::vec2 c) :
	RailLinkCurve(a, b, {c.x, a->pos.y, c.y}, {!c, a->pos, b->pos})
{
}

RailLinkCurve::RailLinkCurve(const NodePtr & a, const NodePtr & b, glm::vec3 c, const Arc arc) :
	RailLink({a, normalize(arc.first + half_pi)}, {b, normalize(arc.second - half_pi)},
			(glm::length(a->pos - c)) * arc_length(arc)),
	centreBase(c), radius {glm::length(ends[0].first->pos - centreBase)}, arc {arc}
{
	const auto & e0p {ends[0].first->pos};
	const auto & e1p {ends[1].first->pos};
	const auto slength = round_sleepers(length / 2.F);
	const auto segs = std::round(5.F * slength / std::pow(radius, 0.7F));
	const auto step {glm::vec3 {arc_length(arc), e0p.y - e1p.y, slength} / segs};
	const auto trans {glm::translate(centreBase)};

	int segCount = segs;
	vertices.reserve((segCount + 1) * railCrossSection.size());
	indices.reserve(segCount * 2 * railCrossSection.size());
	for (glm::vec3 swing = {arc.second, -e1p.y, 0.F}; segCount >= 0; swing += step, --segCount) {
		const auto t {trans * glm::rotate(half_pi - swing.x, up) * glm::translate(glm::vec3 {radius, swing.y, 0.F})};
		for (const auto & rcs : railCrossSection) {
			const glm::vec3 m {(t * glm::vec4 {rcs.first, 1})};
			vertices.emplace_back(m, glm::vec2 {rcs.second, swing.z}, up);
		}
	}
	defaultMesh();
}

Transform
RailLinkCurve::positionAt(float dist, unsigned char start) const
{
	static constexpr std::array<float, 2> dirOffset {half_pi, -half_pi};
	const auto frac {dist / length};
	const auto es {std::make_pair(ends[start].first.get(), ends[1 - start].first.get())};
	const auto as {std::make_pair(arc[start], arc[1 - start])};
	const auto ang {as.first + ((as.second - as.first) * frac)};
	const auto angArc {ang - half_pi};
	const auto relPos {glm::vec3 {std::cos(angArc), 0, -std::sin(angArc)} * radius};
	const auto relClimb {
			glm::vec3 {0, -centreBase.y + es.first->pos.y + ((es.second->pos.y - es.first->pos.y) * frac), 0}};

	return Transform {relPos + relClimb + centreBase, {0, normalize(ang + dirOffset[start]), 0}};
}
