#include "link.h"
#include <compare>
#include <location.hpp>
#include <maths.h>
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
