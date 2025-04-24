#pragma once

#include "config/types.h"
#include "maths.h"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <span>

template<typename PositionType> class Ray {
public:
#ifndef __cpp_aggregate_paren_init
	Ray(PositionType start, Direction3D direction) : start {start}, direction {direction} { }
#endif

	PositionType start;
	Direction3D direction;

	[[nodiscard]] PositionType::value_type
	distanceToLine(const PositionType & p1, const PositionType & e1) const
	{
		// https://en.wikipedia.org/wiki/Skew_lines
		const RelativePosition3D diff = p1 - e1;
		const auto d1 = glm::normalize(diff);
		const auto n = crossProduct(d1, direction);
		const auto n2 = crossProduct(direction, n);
		const auto c1 = p1 + PositionType((glm::dot(RelativePosition3D(start - p1), n2) / glm::dot(d1, n2)) * d1);
		const auto difflength = glm::length(diff);
		if (::distance(c1, p1) > difflength || ::distance(c1, e1) > difflength) {
			return std::numeric_limits<typename PositionType::value_type>::infinity();
		}
		return static_cast<PositionType::value_type>(glm::abs(glm::dot(n, RelativePosition3D(p1 - start))));
	}

	[[nodiscard]] bool
	passesCloseToEdges(const std::span<const PositionType> positions, const PositionType::value_type distance) const
	{
		return std::adjacent_find(positions.begin(), positions.end(), [this, distance](const auto & a, const auto & b) {
			return distanceToLine(a, b) <= distance;
		}) != positions.end();
	}

	bool
	intersectPlane(const PositionType orig, const Direction3D norm, RelativeDistance & distance) const
	{
		if constexpr (std::is_floating_point_v<typename PositionType::value_type>) {
			return glm::intersectRayPlane(start, direction, orig, norm, distance) && distance >= 0.F;
		}
		else {
			const RelativePosition3D origr = orig - start;
			return glm::intersectRayPlane({}, direction, origr, norm, distance) && distance >= 0.F;
		}
	}

	struct IntersectTriangleResult {
		BaryPosition bary;
		RelativeDistance distance;
	};

	std::optional<IntersectTriangleResult>
	intersectTriangle(const PositionType t0, const PositionType t1, const PositionType t2) const
	{
		IntersectTriangleResult out;
		if constexpr (std::is_floating_point_v<typename PositionType::value_type>) {
			if (glm::intersectRayTriangle(start, direction, t0, t1, t2, out.bary, out.distance)
					&& out.distance >= 0.F) {
				return out;
			}
		}
		else {
			const RelativePosition3D t0r = t0 - start, t1r = t1 - start, t2r = t2 - start;
			if (glm::intersectRayTriangle({}, direction, t0r, t1r, t2r, out.bary, out.distance)
					&& out.distance >= 0.F) {
				return out;
			}
		}
		return std::nullopt;
	}

	struct IntersectSphereResult {
		PositionType position;
		Normal3D normal;
	};

	std::optional<IntersectSphereResult>
	intersectSphere(const PositionType centre, const PositionType::value_type size) const
	{
		IntersectSphereResult out;
		if constexpr (std::is_floating_point_v<typename PositionType::value_type>) {
			if (glm::intersectRaySphere(start, direction, centre, size, out.position, out.normal)) {
				return out;
			}
		}
		else {
			const RelativePosition3D cr = centre - start;
			RelativePosition3D positionF {};
			if (glm::intersectRaySphere(
						{}, direction, cr, static_cast<RelativeDistance>(size), positionF, out.normal)) {
				out.position = GlobalPosition3D(positionF) + start;
				return out;
			}
		}
		return std::nullopt;
	}
};

class RayFactory {
public:
	template<typename PositionType>
	static Ray<PositionType>
	fromPoints(PositionType start, PositionType p)
	{
		return {start, glm::normalize(p - start)};
	}
};
