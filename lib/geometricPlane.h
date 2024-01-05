#pragma once

#include "config/types.h"
#include "ray.h"
#include <glm/geometric.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/vec3.hpp>
#include <optional>

class GeometricPlane {
public:
	struct DistAndPosition {
		float dist;
		Position3D position;
	};
	enum class PlaneRelation { Above, Below, On };

	Position3D origin;
	Normal3D normal;

	[[nodiscard]] inline PlaneRelation
	getRelation(Position3D point) const
	{
		const auto d = glm::dot(normal, point - origin);
		return d < 0.F ? PlaneRelation::Below : d > 0.F ? PlaneRelation::Above : PlaneRelation::On;
	}

	[[nodiscard]] inline std::optional<DistAndPosition>
	getRayIntersectPosition(const Ray & ray) const
	{
		float dist {};
		if (!glm::intersectRayPlane(ray.start, ray.direction, origin, normal, dist)) {
			return {};
		}
		return DistAndPosition {dist, ray.start + (ray.direction * dist)};
	}

	inline static bool
	isIntersect(PlaneRelation a, PlaneRelation b)
	{
		return ((a == PlaneRelation::Above && b == PlaneRelation::Below)
				|| (a == PlaneRelation::Below && b == PlaneRelation::Above));
	}
};
