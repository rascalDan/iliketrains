#include "link.h"
#include <compare>
#include <glm/gtx/transform.hpp>
#include <location.hpp>
#include <maths.h>
#include <ray.hpp>
#include <tuple>

Link::Link(End a, End b, float l) : ends {{std::move(a), std::move(b)}}, length {l} { }

LinkCurve::LinkCurve(glm::vec3 c, float r, Arc a) : centreBase {c}, radius {r}, arc {std::move(a)} { }

bool
operator<(const glm::vec3 & a, const glm::vec3 & b)
{
	// NOLINTNEXTLINE(hicpp-use-nullptr,modernize-use-nullptr)
	return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);
}

bool
operator<(const Node & a, const Node & b)
{
	return a.pos < b.pos;
}

Location
LinkStraight::positionAt(float dist, unsigned char start) const
{
	const auto es {std::make_pair(ends[start].node.get(), ends[1 - start].node.get())};
	const auto diff {es.second->pos - es.first->pos};
	const auto dir {glm::normalize(diff)};
	return Location {es.first->pos + vehiclePositionOffset() + dir * dist, {vector_pitch(dir), vector_yaw(dir), 0}};
}

bool
LinkStraight::intersectRay(const Ray & ray) const
{
	return ray.passesCloseToEdges(std::array {ends.front().node->pos, ends.back().node->pos}, 1.F);
}

Location
LinkCurve::positionAt(float dist, unsigned char start) const
{
	static constexpr std::array<float, 2> dirOffset {half_pi, -half_pi};
	const auto frac {dist / length};
	const auto es {std::make_pair(ends[start].node.get(), ends[1 - start].node.get())};
	const auto as {std::make_pair(arc[start], arc[1 - start])};
	const auto ang {as.first + ((as.second - as.first) * frac)};
	const auto relPos {!sincosf(ang) * radius};
	const auto relClimb {vehiclePositionOffset()
			+ glm::vec3 {0, 0, es.first->pos.z - centreBase.z + ((es.second->pos.z - es.first->pos.z) * frac)}};
	const auto pitch {vector_pitch({0, 0, (es.second->pos.z - es.first->pos.z) / length})};
	return Location {relPos + relClimb + centreBase, {pitch, normalize(ang + dirOffset[start]), 0}};
}

bool
LinkCurve::intersectRay(const Ray & ray) const
{
	const auto & e0p {ends[0].node->pos};
	const auto & e1p {ends[1].node->pos};
	const auto slength = round_frac(length / 2.F, 5.F);
	const auto segs = std::round(15.F * slength / std::pow(radius, 0.7F));
	const auto step {glm::vec3 {arc_length(arc), e1p.z - e0p.z, slength} / segs};
	const auto trans {glm::translate(centreBase)};

	auto segCount = static_cast<std::size_t>(std::lround(segs)) + 1;
	std::vector<glm::vec3> points;
	points.reserve(segCount);
	for (glm::vec3 swing = {arc.first, centreBase.z - e0p.z, 0.F}; segCount; swing += step, --segCount) {
		const auto t {trans * glm::rotate(half_pi - swing.x, up) * glm::translate(glm::vec3 {radius, 0.F, swing.y})};
		points.emplace_back(t * glm::vec4 {0, 0, 0, 1});
	}
	return ray.passesCloseToEdges(points, 1.F);
}
