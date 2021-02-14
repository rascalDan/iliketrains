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

RailLinkCurve::RailLinkCurve(const NodePtr & a, const NodePtr & b, glm::vec2 c) :
	RailLinkCurve(a, b, {c.x, a->pos.y, c.y}, {!c, a->pos, b->pos})
{
}

RailLinkCurve::RailLinkCurve(const NodePtr & a, const NodePtr & b, glm::vec3 c, const Arc arc) :
	RailLink({a, normalize(arc.first - half_pi)}, {b, normalize(arc.second + half_pi)},
			(glm::length(a->pos - c)) * arc_length(arc)),
	centreBase(c)
{
	const auto & e0p {ends[0].first->pos};
	const auto & e1p {ends[1].first->pos};
	const auto radius = glm::length(e0p - centreBase);
	const auto slength = round_sleepers(length / 2.F);
	const auto segs = std::round(5.F * slength / std::pow(radius, 0.7F));
	const auto step {glm::vec3 {arc_length(arc), e1p.y - e0p.y, slength} / segs};
	const auto trans {glm::translate(centreBase)};

	int segCount = segs;
	vertices.reserve((segCount + 1) * railCrossSection.size());
	indices.reserve(segCount * 2 * railCrossSection.size());
	for (glm::vec3 swing = {arc.first, 0.F, 0.F}; segCount >= 0; swing += step, --segCount) {
		const auto t {trans * glm::rotate(half_pi - swing.x, up) * glm::translate(glm::vec3 {radius, swing.y, 0.F})};
		for (const auto & rcs : railCrossSection) {
			const glm::vec3 m {(t * glm::vec4 {rcs.first, 1})};
			vertices.emplace_back(m, glm::vec2 {rcs.second, swing.z}, up);
		}
	}
	defaultMesh();
}
