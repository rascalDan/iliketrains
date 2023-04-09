#include "geometricPlane.h"
#include <glm/geometric.hpp>

GeometricPlane::PlaneRelation
GeometricPlane::getRelation(glm::vec3 p) const
{
	const auto d = glm::dot(normal, p - origin);
	return d < 0.f ? PlaneRelation::Below : d > 0.f ? PlaneRelation::Above : PlaneRelation::On;
}
