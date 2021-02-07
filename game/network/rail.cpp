#include "rail.h"
#include "game/network/link.h"
#include <GL/glew.h>
#include <array>
#include <cache.h>
#include <cmath>
#include <gfx/gl/shader.h>
#include <gfx/models/texture.h>
#include <gfx/models/vertex.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <numbers>
#include <type_traits>
#include <utility>

RailLinks::RailLinks() : texture {Texture::cachedTexture.get("rails.jpg")} { }
void RailLinks::tick(TickDuration) { }

static const auto identityModel {glm::identity<glm::mat4>()};

void
RailLinks::render(const Shader & shader) const
{
	shader.setModel(identityModel);
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
constexpr const glm::vec3 up {0, 1, 0};
constexpr const glm::vec3 north {0, 0, 1};
const auto oneeighty {glm::rotate(std::numbers::pi_v<float>, up)};
constexpr auto half_pi {glm::half_pi<float>()};
constexpr auto pi {glm::pi<float>()};
constexpr auto two_pi {glm::two_pi<float>()};
constexpr auto sleepers {5.F}; // There are 5 repetitions of sleepers in the texture

template<typename V>
auto
flat_orientation(const V & diff)
{
	const auto flatdiff {glm::normalize(glm::vec3 {diff.x, 0, diff.z})};
	auto e {glm::orientation(flatdiff, north)};
	// Handle if diff is exactly opposite to north
	return (std::isnan(e[0][0])) ? oneeighty : e;
}

template<typename V>
auto
flat_angle(const V & diff)
{
	const auto flatdiff {glm::normalize(glm::vec3 {diff.x, 0, diff.z})};
	return glm::orientedAngle(flatdiff, north, up);
}

template<typename T>
constexpr auto
round_frac(const T & v, const T & frac)
{
	return std::round(v / frac) * frac;
}

template<typename T>
constexpr auto
round_sleepers(const T & v)
{
	return std::round(v / sleepers) * sleepers;
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

constexpr inline glm::vec3
operator!(const glm::vec2 & v)
{
	return {v.x, 0, v.y};
}

constexpr inline float
arc_length(const Arc & arc)
{
	return arc.second - arc.first;
}

constexpr inline float
normalize(float ang)
{
	while (ang > pi) {
		ang -= two_pi;
	}
	while (ang <= -pi) {
		ang += two_pi;
	}
	return ang;
}

inline Arc
create_arc(const glm::vec3 centre3, glm::vec3 e0p, glm::vec3 e1p)
{
	const auto diffa = centre3 - e0p;
	const auto diffb = centre3 - e1p;
	const auto anga = flat_angle(diffa);
	const auto angb = [&diffb, &anga]() {
		const auto angb = flat_angle(diffb);
		return (angb < anga) ? angb + two_pi : angb;
	}();
	return {anga, angb};
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
