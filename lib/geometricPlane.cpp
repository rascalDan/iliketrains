#include "geometricPlane.h"
#include "ray.h"
#include <glm/geometric.hpp>
#include <glm/gtx/intersect.hpp>

GeometricPlane::PlaneRelation
GeometricPlane::getRelation(Position3D p) const
{
	const auto d = glm::dot(normal, p - origin);
	return d < 0.F ? PlaneRelation::Below : d > 0.F ? PlaneRelation::Above : PlaneRelation::On;
}

bool
GeometricPlane::isIntersect(PlaneRelation a, PlaneRelation b)
{
	return ((a == PlaneRelation::Above && b == PlaneRelation::Below)
			|| (a == PlaneRelation::Below && b == PlaneRelation::Above));
}

std::optional<GeometricPlane::DistAndPosition>
GeometricPlane::getRayIntersectPosition(const Ray & ray) const
{
	float dist {};
	if (!glm::intersectRayPlane(ray.start, ray.direction, origin, normal, dist)) {
		return {};
	}
	return DistAndPosition {dist, ray.start + (ray.direction * dist)};
}
