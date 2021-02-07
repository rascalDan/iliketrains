#include "rail.h"
#include "game/network/link.h"
#include <GL/glew.h>
#include <array>
#include <cache.h>
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
RailLinks::render(const Shader & shader) const
{
	shader.setModel(glm::identity<glm::mat4>());
	texture->Bind();
	links.apply(&RailLink::render, shader);
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
		{{-1.F, 0.F, 0.F}, 0.F},
		{{-.75F, .25F, 0.F}, 0.125F},
		{{.75F, .25F, 0.F}, 0.875F},
		{{1.F, 0.F, 0.F}, 1.F},
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
			if (vertices.size() > railCrossSection.size()) {
				indices.push_back(vertices.size() - railCrossSection.size() - 1);
				indices.push_back(vertices.size() - 1);
			}
		}
	}
	meshes.create<Mesh>(vertices, indices, GL_TRIANGLE_STRIP);
}

RailLinkCurve::RailLinkCurve(const NodePtr & a, const NodePtr & b, glm::vec2 c) :
	RailLinkCurve(a, b, {c.x, a->pos.y, c.y}, create_arc(!c, a->pos, b->pos))
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

	auto addRcs = [this, trans, radius](auto arc) {
		const auto t {trans * glm::rotate(half_pi - arc.x, up) * glm::translate(glm::vec3 {radius, arc.y, 0.F})};
		for (const auto & rcs : railCrossSection) {
			const glm::vec3 m {(t * glm::vec4 {rcs.first, 1})};
			vertices.emplace_back(m, glm::vec2 {rcs.second, arc.z}, up);
		}
	};
	for (glm::vec3 swing = {arc.first, 0.F, 0.F}; swing.x < arc.second; swing += step) {
		addRcs(swing);
	}
	addRcs(glm::vec3 {arc.second, e1p.y - e0p.y, slength});

	for (auto n = 4U; n < vertices.size(); n += 1) {
		indices.push_back(n - 4);
		indices.push_back(n);
	}
	meshes.create<Mesh>(vertices, indices, GL_TRIANGLE_STRIP);
}
