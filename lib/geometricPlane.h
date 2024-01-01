#pragma once

#include "config/types.h"
#include <glm/vec3.hpp>
#include <optional>

class Ray;

class GeometricPlane {
public:
	struct DistAndPosition {
		float dist;
		Position3D position;
	};
	enum class PlaneRelation { Above, Below, On };

	Position3D origin;
	Normal3D normal;

	[[nodiscard]] PlaneRelation getRelation(Position3D point) const;
	[[nodiscard]] std::optional<DistAndPosition> getRayIntersectPosition(const Ray &) const;

	static bool isIntersect(PlaneRelation a, PlaneRelation b);
};
