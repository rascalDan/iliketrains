#pragma once

#include <glm/vec3.hpp>

class GeometricPlane {
public:
	enum class PlaneRelation { Above, Below, On };

	glm::vec3 origin, normal;

	PlaneRelation getRelation(glm::vec3 point) const;
};
