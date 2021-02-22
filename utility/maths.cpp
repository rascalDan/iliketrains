#include "maths.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <initializer_list>
#include <stdexcept>

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

std::pair<glm::vec2, bool>
find_arc_centre(glm::vec2 as, float entrys, glm::vec2 bs, float entrye)
{
	if (as == bs) {
		return {as, false};
	}
	for (const auto lr : {1.F, -1.F}) { // left or right turn (maybe possible with removal of positve check below)
		const auto perps = entrys + (half_pi * lr);
		const auto perpe = entrye - (half_pi * lr);
		const glm::vec2 ad {std::sin(perps), std::cos(perps)};
		const glm::vec2 bd {std::sin(perpe), std::cos(perpe)};

		const auto dx = bs.x - as.x;
		const auto dy = bs.y - as.y;
		const auto det = bd.x * ad.y - bd.y * ad.x;
		if (det != 0) { // near parallel line will yield noisy results
			const auto u = (dy * bd.x - dx * bd.y) / det;
			const auto v = (dy * ad.x - dx * ad.y) / det;
			if (u >= 0 && v >= 0) {
				return {as + ad * u, lr < 0};
			}
		}
	}
	throw std::runtime_error("no intersection");
}
