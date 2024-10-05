#include "maths.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

Arc::Arc(const RelativePosition2D & dir0, const RelativePosition2D & dir1) :
	Arc {vector_yaw(dir0), vector_yaw(dir1)} { }

Arc::Arc(const Angle anga, const Angle angb) : pair {anga, (angb < anga) ? angb + two_pi : angb} { }

glm::mat4
flat_orientation(const Direction3D & diff)
{
	static const auto oneeighty {glm::rotate(pi, up)};
	const auto flatdiff {glm::normalize(diff.xy() || 0.F)};
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
	return rotate_yp(a.y, a.x);
}

glm::mat4
rotate_yp(Angle yaw, Angle pitch)
{
	return rotate_yaw(yaw) * rotate_pitch(pitch);
}

float
vector_yaw(const Direction2D & diff)
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
