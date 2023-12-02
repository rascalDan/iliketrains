#pragma once

#include "config/types.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <numeric>
#include <utility>

struct Arc : public std::pair<float, float> {
	using std::pair<float, float>::pair;

	Arc(const Position3D & centre3, const Position3D & e0p, const Position3D & e1p);

	float
	operator[](unsigned int i) const
	{
		return i ? second : first;
	}
};

constexpr const Position3D origin {0, 0, 0};
constexpr const Position3D up {0, 0, 1};
constexpr const Position3D down {0, 0, -1};
constexpr const Position3D north {0, 1, 0};
constexpr const Position3D south {0, -1, 0};
constexpr const Position3D east {1, 0, 0};
constexpr const Position3D west {-1, 0, 0};
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

template<std::integral T, glm::qualifier Q>
inline constexpr glm::vec<3, T, Q>
crossInt(const glm::vec<3, T, Q> a, const glm::vec<3, T, Q> b)
{
	return {
			(a.y * b.z) - (a.z * b.y),
			(a.z * b.x) - (a.x * b.z),
			(a.x * b.y) - (a.y * b.x),
	};
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

constexpr inline Position2D
operator!(const Position3D & v)
{
	return {v.x, v.y};
}

constexpr inline Position3D
operator^(const Position2D & v, float z)
{
	return {v.x, v.y, z};
}

constexpr inline glm::vec4
operator^(const Position3D & v, float w)
{
	return {v.x, v.y, v.z, w};
}

constexpr inline Position3D
operator!(const Position2D & v)
{
	return v ^ 0.F;
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

inline Position3D
operator%(const Position3D & p, const glm::mat4 & mutation)
{
	const auto p2 = mutation * (p ^ 1);
	return p2 / p2.w;
}

inline Position3D
operator%=(Position3D & p, const glm::mat4 & mutation)
{
	return p = p % mutation;
}

constexpr inline float
arc_length(const Arc & arc)
{
	return arc.second - arc.first;
}

float normalize(float ang);

std::pair<Position2D, bool> find_arc_centre(Position2D start, float entrys, Position2D end, float entrye);
std::pair<Position2D, bool> find_arc_centre(Position2D start, Position2D ad, Position2D end, Position2D bd);
std::pair<float, float> find_arcs_radius(Position2D start, float entrys, Position2D end, float entrye);
float find_arcs_radius(Position2D start, Position2D ad, Position2D end, Position2D bd);

template<typename T>
auto
midpoint(const std::pair<T, T> & v)
{
	return std::midpoint(v.first, v.second);
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
