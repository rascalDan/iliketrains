#include "link.h"
#include <glm/gtx/transform.hpp>
#include <location.h>
#include <maths.h>
#include <ray.h>
#include <tuple>

Link::Link(End endA, End endB, float len) : ends {{std::move(endA), std::move(endB)}}, length {len} { }

LinkCurve::LinkCurve(GlobalPosition3D centre, RelativeDistance radius, Arc arc) :
	centreBase {centre}, radius {radius}, arc {std::move(arc)}
{
}

bool
operator<(const GlobalPosition3D & left, const GlobalPosition3D & right)
{
	// NOLINTNEXTLINE(hicpp-use-nullptr,modernize-use-nullptr)
	return std::tie(left.x, left.y, left.z) < std::tie(right.x, right.y, right.z);
}

bool
operator<(const Node & left, const Node & right)
{
	return left.pos < right.pos;
}

Location
LinkStraight::positionAt(RelativeDistance dist, unsigned char start) const
{
	const auto endNodes = std::make_pair(ends[start].node.get(), ends[1 - start].node.get());
	const auto diff = ::difference(endNodes.second->pos, endNodes.first->pos);
	const auto directionVector = glm::normalize(diff);
	return Location {
			.pos = endNodes.first->pos + (vehiclePositionOffset() + directionVector * dist),
			.rot = {vector_pitch(directionVector), vector_yaw(directionVector), 0},
	};
}

bool
LinkStraight::intersectRay(const Ray<GlobalPosition3D> & ray) const
{
	static constexpr auto PROXIMITY = 1'000;
	return ray.passesCloseToEdges(std::array {ends.front().node->pos, ends.back().node->pos}, PROXIMITY);
}

std::vector<GlobalPosition3D>
LinkStraight::getBase(RelativeDistance width) const
{
	const auto start = ends.front().node->pos;
	const auto end = ends.back().node->pos;
	const auto direction = (vector_normal(normalize(::difference(start, end).xy())) * width / 2.F) || 0.F;
	return {
			start - direction,
			start + direction,
			end - direction,
			end + direction,
	};
}

Location
LinkCurve::positionAt(float dist, unsigned char start) const
{
	static constexpr std::array DIR_OFFSET {half_pi, -half_pi};
	const auto frac = dist / length;
	const auto endNodes = std::make_pair(ends[start].node.get(), ends[1 - start].node.get());
	const auto arcEndAngles = std::make_pair(arc[start], arc[1 - start]);
	const auto ang = glm::mix(arcEndAngles.first, arcEndAngles.second, frac);
	const auto relPos = (sincos(ang) || 0.F) * radius;
	const auto relClimb = vehiclePositionOffset()
			+ RelativePosition3D {0, 0,
					static_cast<RelativeDistance>(endNodes.first->pos.z - centreBase.z)
							+ (static_cast<RelativeDistance>(endNodes.second->pos.z - endNodes.first->pos.z) * frac)};
	const auto pitch {vector_pitch(difference(endNodes.second->pos, endNodes.first->pos) / length)};
	return Location {
			.pos = GlobalPosition3D(relPos + relClimb) + centreBase,
			.rot = {pitch, normalize(ang + DIR_OFFSET[start]), 0},
	};
}

bool
LinkCurve::intersectRay(const Ray<GlobalPosition3D> & ray) const
{
	const auto e0p = ends[0].node->pos.z;
	const auto e1p = ends[1].node->pos.z;
	const auto slength = round_frac(length / 2.F, 5.F);
	const auto segs = std::round(15.F * slength / std::pow(radius, 0.7F));
	const auto step {glm::vec<2, RelativeDistance> {arc.length(), e1p - e0p} / segs};

	auto segCount = static_cast<std::size_t>(std::lround(segs)) + 1;
	std::vector<GlobalPosition3D> points;
	points.reserve(segCount);
	for (std::remove_const_t<decltype(step)> swing = {arc.first, centreBase.z - e0p}; segCount;
			swing += step, --segCount) {
		points.emplace_back(centreBase + ((sincos(swing.x) * radius) || swing.y));
	}
	return ray.passesCloseToEdges(points, 1.F);
}

std::vector<GlobalPosition3D>
LinkCurve::getBase(RelativeDistance width) const
{
	const auto start = ends.front().node->pos;
	const auto end = ends.back().node->pos;
	const auto segs = std::ceil(std::sqrt(radius) * 0.02F * arc.length());
	const auto step = glm::vec<2, RelativeDistance> {arc.length(), end.z - start.z} / segs;

	auto segCount = static_cast<size_t>(segs) + 1;
	std::vector<GlobalPosition3D> out;
	out.reserve(segCount);
	for (RelativePosition2D swing = {arc.first, centreBase.z - start.z}; segCount != 0U; swing += step, --segCount) {
		const auto direction = sincos(swing.x);
		const auto linkCentre = centreBase + ((direction * radius) || swing.y);
		const auto toEdge = (direction * width / 2.F) || 0.F;
		out.emplace_back(linkCentre + toEdge);
		out.emplace_back(linkCentre - toEdge);
	}
	return out;
}
