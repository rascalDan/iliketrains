#pragma once

#include "config/types.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <numeric>
#include <stdexcept>
#include <utility>

struct Arc : public std::pair<float, float> {
	using std::pair<float, float>::pair;

	template<typename T, glm::qualifier Q>
	Arc(const glm::vec<3, T, Q> & centre3, const glm::vec<3, T, Q> & e0p, const glm::vec<3, T, Q> & e1p);

	auto
	operator[](bool i) const
	{
		return i ? second : first;
	}
};

constexpr const RelativePosition3D up {0, 0, 1};
constexpr const RelativePosition3D down {0, 0, -1};
constexpr const RelativePosition3D north {0, 1, 0};
constexpr const RelativePosition3D south {0, -1, 0};
constexpr const RelativePosition3D east {1, 0, 0};
constexpr const RelativePosition3D west {-1, 0, 0};
constexpr auto half_pi {glm::half_pi<float>()};
constexpr auto quarter_pi {half_pi / 2};
constexpr auto pi {glm::pi<float>()};
constexpr auto two_pi {glm::two_pi<float>()};

glm::mat4 flat_orientation(const Rotation3D & diff);

// C++ wrapper for C's sincosf, but with references, not pointers
inline auto
sincosf(float a, float & s, float & c)
{
	return sincosf(a, &s, &c);
}

inline Rotation2D
sincosf(float a)
{
	Rotation2D sc;
	sincosf(a, sc.x, sc.y);
	return sc;
}

glm::mat2 rotate_flat(float);
glm::mat4 rotate_roll(float);
glm::mat4 rotate_yaw(float);
glm::mat4 rotate_pitch(float);
glm::mat4 rotate_yp(Rotation2D);
glm::mat4 rotate_ypr(Rotation3D);

float vector_yaw(const Direction3D & diff);
float vector_pitch(const Direction3D & diff);

float round_frac(const float & v, const float & frac);

template<typename T>
inline constexpr auto
sq(T v)
{
	return v * v;
}

template<glm::qualifier Q>
inline constexpr glm::vec<3, int64_t, Q>
crossProduct(const glm::vec<3, int64_t, Q> a, const glm::vec<3, int64_t, Q> b)
{
	return {
			(a.y * b.z) - (a.z * b.y),
			(a.z * b.x) - (a.x * b.z),
			(a.x * b.y) - (a.y * b.x),
	};
}

template<std::integral T, glm::qualifier Q>
inline constexpr glm::vec<3, T, Q>
crossProduct(const glm::vec<3, T, Q> a, const glm::vec<3, T, Q> b)
{
	return crossProduct<int64_t, Q>(a, b);
}

template<std::floating_point T, glm::qualifier Q>
inline constexpr glm::vec<3, T, Q>
crossProduct(const glm::vec<3, T, Q> a, const glm::vec<3, T, Q> b)
{
	return glm::cross(a, b);
}

template<typename R = float, typename Ta, typename Tb>
inline constexpr auto
ratio(Ta a, Tb b)
{
	return (static_cast<R>(a) / static_cast<R>(b));
}

template<typename R = float, typename T, glm::qualifier Q>
inline constexpr auto
ratio(glm::vec<2, T, Q> v)
{
	return ratio<R>(v.x, v.y);
}

template<glm::length_t L = 3, typename T, glm::qualifier Q>
inline constexpr glm::vec<L, T, Q>
perspective_divide(glm::vec<4, T, Q> v)
{
	return v / v.w;
}

template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q>
inline constexpr glm::vec<L1 + L2, T, Q>
operator||(const glm::vec<L1, T, Q> v1, const glm::vec<L2, T, Q> v2)
{
	return {v1, v2};
}

template<glm::length_t L, typename T, glm::qualifier Q>
inline constexpr glm::vec<L + 1, T, Q>
operator||(const glm::vec<L, T, Q> v1, const T v2)
{
	return {v1, v2};
}

template<glm::length_t L, typename T, glm::qualifier Q>
inline constexpr glm::vec<L, T, Q>
operator%(const glm::vec<L, T, Q> & p, const glm::mat<L + 1, L + 1, T, Q> & mutation)
{
	const auto p2 = mutation * (p || T(1));
	return p2 / p2.w;
}

template<glm::length_t L, typename T, glm::qualifier Q>
inline constexpr glm::vec<L, T, Q>
operator%=(glm::vec<L, T, Q> & p, const glm::mat<L + 1, L + 1, T, Q> & mutation)
{
	return p = p % mutation;
}

constexpr inline float
arc_length(const Arc & arc)
{
	return arc.second - arc.first;
}

float normalize(float ang);

template<typename T, glm::qualifier Q>
std::pair<glm::vec<2, T, Q>, bool>
find_arc_centre(glm::vec<2, T, Q> start, Rotation2D startDir, glm::vec<2, T, Q> end, Rotation2D endDir)
{
	const auto det = endDir.x * startDir.y - endDir.y * startDir.x;
	if (det != 0) { // near parallel line will yield noisy results
		const auto d = end - start;
		const auto u = (d.y * endDir.x - d.x * endDir.y) / det;
		return {start + startDir * u, u < 0};
	}
	throw std::runtime_error("no intersection");
}

template<typename T, glm::qualifier Q>
std::pair<glm::vec<2, T, Q>, bool>
find_arc_centre(glm::vec<2, T, Q> start, Angle entrys, glm::vec<2, T, Q> end, Angle entrye)
{
	if (start == end) {
		return {start, false};
	}
	return find_arc_centre(start, sincosf(entrys + half_pi), end, sincosf(entrye - half_pi));
}

template<typename T, glm::qualifier Q>
Angle
find_arcs_radius(glm::vec<2, T, Q> start, Rotation2D ad, glm::vec<2, T, Q> end, Rotation2D bd)
{
	using std::sqrt;

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

template<typename T, glm::qualifier Q>
std::pair<Angle, Angle>
find_arcs_radius(glm::vec<2, T, Q> start, Angle entrys, glm::vec<2, T, Q> end, Angle entrye)
{
	const auto getrad = [&](auto leftOrRight) {
		return find_arcs_radius(start, sincosf(entrys + leftOrRight), end, sincosf(entrye + leftOrRight));
	};
	return {getrad(-half_pi), getrad(half_pi)};
}

template<typename T>
auto
midpoint(const std::pair<T, T> & v)
{
	return std::midpoint(v.first, v.second);
}

template<typename T, glm::qualifier Q>
Arc::Arc(const glm::vec<3, T, Q> & centre3, const glm::vec<3, T, Q> & e0p, const glm::vec<3, T, Q> & e1p) :
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

// Conversions
template<typename T>
inline constexpr auto
mph_to_ms(T v)
{
	return v / 2.237L;
}

template<typename T>
inline constexpr auto
kph_to_ms(T v)
{
	return v / 3.6L;
}

// ... literals are handy for now, probably go away when we load stuff externally
float operator"" _mph(const long double v);
float operator"" _kph(const long double v);
