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

template<RailLinkConcept T>
std::shared_ptr<T>
RailLinks::addLink(glm::vec3 a, glm::vec3 b)
{
	const auto node1 = *nodes.insert(std::make_shared<Node>(a)).first;
	const auto node2 = *nodes.insert(std::make_shared<Node>(b)).first;
	// TODO set end flag properly
	return links.create<T>(Link::End {node1, true}, Link::End {node2, true});
}

template<RailLinkConcept T>
std::shared_ptr<T>
RailLinks::addLink(glm::vec3 a, glm::vec3 b, glm::vec2 centre)
{
	const auto node1 = *nodes.insert(std::make_shared<Node>(a)).first;
	const auto node2 = *nodes.insert(std::make_shared<Node>(b)).first;
	// TODO set end flag properly
	return links.create<T>(Link::End {node1, true}, Link::End {node2, true}, centre);
}

template std::shared_ptr<RailLinkStraight> RailLinks::addLink(glm::vec3, glm::vec3);
template std::shared_ptr<RailLinkCurve> RailLinks::addLink(glm::vec3, glm::vec3, glm::vec2);

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

RailLinkStraight::RailLinkStraight(End a, End b) : RailLink(std::move(a), std::move(b))
{
	vertices.reserve(2 * railCrossSection.size());
	indices.reserve(2 * railCrossSection.size());
	const auto diff {ends[1].first->pos - ends[0].first->pos};
	const auto len = round_sleepers(glm::length(diff) / 2.F);
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

RailLinkCurve::RailLinkCurve(End a, End b, glm::vec2 c) : RailLink(std::move(a), std::move(b)), centre(c)
{
	const auto & e0p {ends[0].first->pos};
	const auto & e1p {ends[1].first->pos};
	const glm::vec3 centre3 {centre.x, e0p.y, centre.y};
	const auto diffa = centre3 - e0p;
	const auto diffb = centre3 - e1p;
	const auto anga = glm::orientedAngle(glm::normalize(diffa), north, up) - half_pi;
	const auto angb = [&diffb, &anga]() {
		const auto angb = glm::orientedAngle(glm::normalize(diffb), north, up) - half_pi;
		return (angb < anga) ? angb + glm::radians(360.f) : angb;
	}();
	const auto radius = glm::length(e0p - centre3);
	const auto length = round_sleepers(radius * (angb - anga) / 2.F);
	const auto step {glm::vec3 {std::abs(angb - anga), e1p.y - e0p.y, length} / std::round(angb - anga) / 5.F};
	const auto trans {glm::translate(centre3)};

	auto addRcs = [this, trans, radius](auto arc) {
		const auto t {trans * glm::rotate(arc.x, up) * glm::translate(glm::vec3 {radius, arc.y, 0.F})};
		for (const auto & rcs : railCrossSection) {
			const glm::vec3 m {(t * glm::vec4 {rcs.first, 1})};
			vertices.emplace_back(m, glm::vec2 {rcs.second, arc.z}, up);
		}
	};
	for (glm::vec3 arc = {anga, 0.F, 0.F}; arc.x < angb; arc += step) {
		addRcs(arc);
	}
	addRcs(glm::vec3 {angb, e1p.y - e0p.y, length});

	for (auto n = 4U; n < vertices.size(); n += 1) {
		indices.push_back(n);
		indices.push_back(n - 4);
	}
	meshes.create<Mesh>(vertices, indices, GL_TRIANGLE_STRIP);
}
