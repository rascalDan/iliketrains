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
vector_yaw(const glm::vec3 & diff)
{
	return std::atan2(diff.x, diff.z);
}

float
vector_pitch(const glm::vec3 & diff)
{
	return std::atan(diff.y);
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

Arc::Arc(const glm::vec3 & centre3, const glm::vec3 & e0p, const glm::vec3 & e1p) :
	Arc([&]() -> Arc {
		const auto diffa = e0p - centre3;
		const auto diffb = e1p - centre3;
		const auto anga = vector_yaw(diffa);
		const auto angb = [&diffb, &anga]() {
			const auto angb = vector_yaw(diffb);
			return (angb < anga) ? angb + two_pi : angb;
		}();
		return {anga, angb};
	}())
{
}
