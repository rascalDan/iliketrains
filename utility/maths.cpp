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

float
find_arcs_radius(glm::vec2 start, glm::vec2 ad, glm::vec2 end, glm::vec2 bd)
{
	// Short name functions for big forula
	auto sq = [](auto v) {
		return v * v;
	};
	auto sqrt = [](float v) {
		return std::sqrt(v);
	};

	// Calculates path across both arcs along the normals... pythagorean theorem... for some known radius r
	// (2r)^2 = ((m + (X*r)) - (o + (Z*r)))^2 + ((n + (Y*r)) - (p + (W*r)))^2
	// According to symbolabs.com equation tool, that solves for r to give:
	// r=(-2 m X+2 X o+2 m Z-2 o Z-2 n Y+2 Y p+2 n W-2 p W-sqrt((2 m X-2 X o-2 m Z+2 o Z+2 n Y-2 Y p-2 n W+2 p W)^(2)-4
	// (X^(2)-2 X Z+Z^(2)+Y^(2)-2 Y W+W^(2)-4) (m^(2)-2 m o+o^(2)+n^(2)-2 n p+p^(2))))/(2 (X^(2)-2 X Z+Z^(2)+Y^(2)-2 Y
	// W+W^(2)-4))

	// These exist cos limitations of online formula rearrangement, and I'm OK with that.
	const auto &m {start.x}, &n {start.y}, &o {end.x}, &p {end.y};
	const auto &X {ad.x}, &Y {ad.y}, &Z {bd.x}, &W {bd.y};

	return (2 * m * X - 2 * X * o - 2 * m * Z + 2 * o * Z + 2 * n * Y - 2 * Y * p - 2 * n * W + 2 * p * W
				   - sqrt(sq(-2 * m * X + 2 * X * o + 2 * m * Z - 2 * o * Z - 2 * n * Y + 2 * Y * p + 2 * n * W
								  - 2 * p * W)
						   - (4 * (sq(X) - 2 * X * Z + sq(Z) + sq(Y) - 2 * Y * W + sq(W) - 4)
								   * (sq(m) - 2 * m * o + sq(o) + sq(n) - 2 * n * p + sq(p)))))
			/ (2 * (sq(X) - 2 * X * Z + sq(Z) + sq(Y) - 2 * Y * W + sq(W) - 4));
}
