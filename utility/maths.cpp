#include "maths.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
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
	const auto perps = entrys + half_pi;
	const auto perpe = entrye - half_pi;
	const glm::vec2 ad {std::sin(perps), std::cos(perps)};
	const glm::vec2 bd {std::sin(perpe), std::cos(perpe)};
	return find_arc_centre(as, ad, bs, bd);
}

std::pair<glm::vec2, bool>
find_arc_centre(glm::vec2 as, glm::vec2 ad, glm::vec2 bs, glm::vec2 bd)
{
	const auto det = bd.x * ad.y - bd.y * ad.x;
	if (det != 0) { // near parallel line will yield noisy results
		const auto d = bs - as;
		const auto u = (d.y * bd.x - d.x * bd.y) / det;
		return {as + ad * u, u < 0};
	}
	throw std::runtime_error("no intersection");
}
