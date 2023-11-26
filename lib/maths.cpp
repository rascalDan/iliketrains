#include "maths.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <stdexcept>

glm::mat4
flat_orientation(const Direction3D & diff)
{
	static const auto oneeighty {glm::rotate(pi, up)};
	const auto flatdiff {glm::normalize(!!diff)};
	auto e {glm::orientation(flatdiff, north)};
	// Handle if diff is exactly opposite to north
	return (std::isnan(e[0][0])) ? oneeighty : e;
}

// Helper to lookup into a matrix given an xy vector coordinate
template<typename M, typename I>
inline auto &
operator^(M & m, glm::vec<2, I> xy)
{
	return m[xy.x][xy.y];
}

// Create a matrix for the angle, given the targets into the matrix
template<typename M, typename I>
inline auto
rotation(typename M::value_type a, glm::vec<2, I> c1, glm::vec<2, I> s1, glm::vec<2, I> c2, glm::vec<2, I> ms2)
{
	M m(1);
	sincosf(a, m ^ s1, m ^ c1);
	m ^ c2 = m ^ c1;
	m ^ ms2 = -(m ^ s1);
	return m;
}

// Create a flat (2D) transformation matrix
glm::mat2
rotate_flat(float a)
{
	return rotation<glm::mat2, glm::length_t>(a, {0, 0}, {0, 1}, {1, 1}, {1, 0});
}

// Create a yaw transformation matrix
glm::mat4
rotate_yaw(float a)
{
	return rotation<glm::mat4, glm::length_t>(a, {0, 0}, {1, 0}, {1, 1}, {0, 1});
}

// Create a roll transformation matrix
glm::mat4
rotate_roll(float a)
{
	return rotation<glm::mat4, glm::length_t>(a, {0, 0}, {2, 0}, {2, 2}, {0, 2});
}

// Create a pitch transformation matrix
glm::mat4
rotate_pitch(float a)
{
	return rotation<glm::mat4, glm::length_t>(a, {1, 1}, {1, 2}, {2, 2}, {2, 1});
}

// Create a combined yaw, pitch, roll transformation matrix
glm::mat4
rotate_ypr(Rotation3D a)
{
	return rotate_yaw(a.y) * rotate_pitch(a.x) * rotate_roll(a.z);
}

glm::mat4
rotate_yp(Rotation2D a)
{
	return rotate_yaw(a.y) * rotate_pitch(a.x);
}

float
vector_yaw(const Direction3D & diff)
{
	return std::atan2(diff.x, diff.y);
}

float
vector_pitch(const Direction3D & diff)
{
	return std::atan(diff.z);
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

Arc::Arc(const Position3D & centre3, const Position3D & e0p, const Position3D & e1p) :
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

std::pair<Position2D, bool>
find_arc_centre(Position2D as, float entrys, Position2D bs, float entrye)
{
	if (as == bs) {
		return {as, false};
	}
	return find_arc_centre(as, sincosf(entrys + half_pi), bs, sincosf(entrye - half_pi));
}

std::pair<Position2D, bool>
find_arc_centre(Position2D as, Position2D ad, Position2D bs, Position2D bd)
{
	const auto det = bd.x * ad.y - bd.y * ad.x;
	if (det != 0) { // near parallel line will yield noisy results
		const auto d = bs - as;
		const auto u = (d.y * bd.x - d.x * bd.y) / det;
		return {as + ad * u, u < 0};
	}
	throw std::runtime_error("no intersection");
}

std::pair<float, float>
find_arcs_radius(Position2D start, float entrys, Position2D end, float entrye)
{
	const auto getrad = [&](float leftOrRight) {
		return find_arcs_radius(start, sincosf(entrys + leftOrRight), end, sincosf(entrye + leftOrRight));
	};
	return {getrad(-half_pi), getrad(half_pi)};
}

float
find_arcs_radius(Position2D start, Position2D ad, Position2D end, Position2D bd)
{
	// Short name functions for big forula
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

float
operator"" _mph(const long double v)
{
	return static_cast<float>(mph_to_ms(v));
}

float
operator"" _kph(const long double v)
{
	return static_cast<float>(kph_to_ms(v));
}
