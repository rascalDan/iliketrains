#pragma once

#include "config/types.h"
#include "ray.h"
#include <glm/geometric.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/vec3.hpp>
#include <optional>

class GeometricPlane {
public:
	enum class PlaneRelation { Above, Below, On };

	static bool isIntersect(PlaneRelation a, PlaneRelation b);
};

template<typename PositionType> class GeometricPlaneT : public GeometricPlane {
public:
	struct DistAndPosition {
		PositionType::value_type dist;
		PositionType position;
	};

	PositionType origin;
	Normal3D normal;

	[[nodiscard]] inline PlaneRelation
	getRelation(PositionType point) const
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
};
