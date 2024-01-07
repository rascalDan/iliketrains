#include "geometricPlane.h"

bool
GeometricPlane::isIntersect(PlaneRelation a, PlaneRelation b)
{
	return ((a == PlaneRelation::Above && b == PlaneRelation::Below)
			|| (a == PlaneRelation::Below && b == PlaneRelation::Above));
}
