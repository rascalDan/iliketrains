#include "maths.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

glm::mat4
flat_orientation(const glm::vec3 & diff)
{
	static const auto oneeighty {glm::rotate(pi, up)};
	const auto flatdiff {glm::normalize(glm::vec3 {diff.x, 0, diff.z})};
	auto e {glm::orientation(flatdiff, north)};
	// Handle if diff is exactly opposite to north
	return (std::isnan(e[0][0])) ? oneeighty : e;
}

float
flat_angle(const glm::vec3 & diff)
{
	const auto flatdiff {glm::normalize(glm::vec3 {diff.x, 0, diff.z})};
	return glm::orientedAngle(flatdiff, north, up);
}

float
round_frac(const float & v, const float & frac)
{
	return std::round(v / frac) * frac;
}

float
normalize(float ang)
{
	while (ang > pi) {
		ang -= two_pi;
	}
	while (ang <= -pi) {
		ang += two_pi;
	}
	return ang;
}

Arc
create_arc(const glm::vec3 & centre3, const glm::vec3 & e0p, const glm::vec3 & e1p)
{
	const auto diffa = centre3 - e0p;
	const auto diffb = centre3 - e1p;
	const auto anga = flat_angle(diffa);
	const auto angb = [&diffb, &anga]() {
		const auto angb = flat_angle(diffb);
		return (angb < anga) ? angb + two_pi : angb;
	}();
	return {anga, angb};
}
