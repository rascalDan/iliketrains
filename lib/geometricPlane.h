#pragma once

#include <glm/vec3.hpp>
#include <optional>

class Ray;

class GeometricPlane {
public:
	struct DistAndPosition {
		float dist;
		glm::vec3 position;
	};
	enum class PlaneRelation { Above, Below, On };

	glm::vec3 origin, normal;

	PlaneRelation getRelation(glm::vec3 point) const;
	std::optional<DistAndPosition> getRayIntersectPosition(const Ray &) const;

	static bool isIntersect(PlaneRelation a, PlaneRelation b);
};
