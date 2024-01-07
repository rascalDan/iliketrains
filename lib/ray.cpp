#include "ray.h"
#include "maths.h"
#include <algorithm>

Ray
Ray::fromPoints(Position3D start, Position3D p)
{
	return {start, glm::normalize(p - start)};
}

float
Ray::distanceToLine(const Position3D & p1, const Position3D & e1) const
{
	// https://en.wikipedia.org/wiki/Skew_lines
	const auto diff = p1 - e1;
	const auto d1 = glm::normalize(diff);
	const auto &p2 = start, &d2 = direction;
	const auto n = crossProduct(d1, d2);
	const auto n2 = crossProduct(d2, n);
	const auto c1 = p1 + (glm::dot((p2 - p1), n2) / glm::dot(d1, n2)) * d1;
	const auto difflength = glm::length(diff);
	if (glm::length(c1 - p1) > difflength || glm::length(c1 - e1) > difflength) {
		return std::numeric_limits<float>::infinity();
	}
	return glm::abs(glm::dot(n, p1 - p2));
}

bool
Ray::passesCloseToEdges(const std::span<const Position3D> positions, float distance) const
{
	return std::adjacent_find(positions.begin(), positions.end(),
				   [this, distance](const Position3D & a, const Position3D & b) {
					   return distanceToLine(a, b) <= distance;
				   })
			!= positions.end();
}
