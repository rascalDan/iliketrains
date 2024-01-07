#include "link.h"
#include <glm/gtx/transform.hpp>
#include <location.h>
#include <maths.h>
#include <ray.h>
#include <tuple>

Link::Link(End a, End b, float l) : ends {{std::move(a), std::move(b)}}, length {l} { }

LinkCurve::LinkCurve(GlobalPosition3D c, RelativeDistance r, Arc a) : centreBase {c}, radius {r}, arc {std::move(a)} { }

bool
operator<(const GlobalPosition3D & a, const GlobalPosition3D & b)
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
LinkStraight::positionAt(RelativeDistance dist, unsigned char start) const
{
	const auto es {std::make_pair(ends[start].node.get(), ends[1 - start].node.get())};
	const RelativePosition3D diff {es.second->pos - es.first->pos};
	const auto dir {glm::normalize(diff)};
	return Location {es.first->pos + GlobalPosition3D(vehiclePositionOffset() + dir * dist),
			{vector_pitch(dir), vector_yaw(dir), 0}};
}

bool
LinkStraight::intersectRay(const Ray<GlobalPosition3D> & ray) const
{
	return ray.passesCloseToEdges(
			std::array {GlobalPosition3D {ends.front().node->pos}, GlobalPosition3D {ends.back().node->pos}}, 1000);
}

Location
LinkCurve::positionAt(float dist, unsigned char start) const
{
	static constexpr std::array<float, 2> dirOffset {half_pi, -half_pi};
	const auto frac {dist / length};
	const auto es {std::make_pair(ends[start].node.get(), ends[1 - start].node.get())};
	const auto as {std::make_pair(arc[start], arc[1 - start])};
	const auto ang {as.first + ((as.second - as.first) * frac)};
	const auto relPos {(sincosf(ang) || 0.F) * radius};
	const auto relClimb {vehiclePositionOffset()
			+ RelativePosition3D {0, 0,
					static_cast<RelativeDistance>(es.first->pos.z - centreBase.z)
							+ (static_cast<RelativeDistance>(es.second->pos.z - es.first->pos.z) * frac)}};
	const auto pitch {vector_pitch({0, 0, static_cast<RelativeDistance>(es.second->pos.z - es.first->pos.z) / length})};
	return Location {GlobalPosition3D(relPos + relClimb) + centreBase, {pitch, normalize(ang + dirOffset[start]), 0}};
}

bool
LinkCurve::intersectRay(const Ray<GlobalPosition3D> & ray) const
{
	const auto & e0p {ends[0].node->pos};
	const auto & e1p {ends[1].node->pos};
	const auto slength = round_frac(length / 2.F, 5.F);
	const auto segs = std::round(15.F * slength / std::pow(radius, 0.7F));
	const auto step {Position2D {arc_length(arc), e1p.z - e0p.z} / segs};

	auto segCount = static_cast<std::size_t>(std::lround(segs)) + 1;
	std::vector<GlobalPosition3D> points;
	points.reserve(segCount);
	for (Position2D swing = {arc.first, centreBase.z - e0p.z}; segCount; swing += step, --segCount) {
		points.emplace_back(centreBase + GlobalPosition3D((sincosf(swing.x) * radius) || swing.y));
	}
	return ray.passesCloseToEdges(points, 1.F);
}
