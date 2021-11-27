#ifndef MATH_H
#define MATH_H

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <utility>

struct Arc : public std::pair<float, float> {
	using std::pair<float, float>::pair;

	Arc(const glm::vec3 & centre3, const glm::vec3 & e0p, const glm::vec3 & e1p);

	float
	operator[](unsigned int i) const
	{
		return i ? second : first;
	}
};

constexpr const glm::vec3 origin {0, 0, 0};
constexpr const glm::vec3 up {0, 0, 1};
constexpr const glm::vec3 down {0, 0, -1};
constexpr const glm::vec3 north {0, 1, 0};
constexpr const glm::vec3 south {0, -1, 0};
constexpr const glm::vec3 east {1, 0, 0};
constexpr const glm::vec3 west {-1, 0, 0};
constexpr auto half_pi {glm::half_pi<float>()};
constexpr auto quarter_pi {half_pi / 2};
constexpr auto pi {glm::pi<float>()};
constexpr auto two_pi {glm::two_pi<float>()};

glm::mat4 flat_orientation(const glm::vec3 & diff);

// C++ wrapper for C's sincosf, but with references, not pointers
inline auto
sincosf(float a, float & s, float & c)
{
	return sincosf(a, &s, &c);
}

inline glm::vec2
sincosf(float a)
{
	glm::vec2 sc;
	sincosf(a, sc.x, sc.y);
	return sc;
}

glm::mat2 rotate_flat(float);
glm::mat4 rotate_roll(float);
glm::mat4 rotate_yaw(float);
glm::mat4 rotate_pitch(float);
glm::mat4 rotate_ypr(glm::vec3);

float vector_yaw(const glm::vec3 & diff);
float vector_pitch(const glm::vec3 & diff);

float round_frac(const float & v, const float & frac);

template<typename T>
inline constexpr auto
sq(T v)
{
	return v * v;
}

template<typename R = float, typename Ta, typename Tb>
inline constexpr auto
rdiv(Ta a, Tb b)
{
	return (static_cast<R>(a) / static_cast<R>(b));
}

constexpr inline glm::vec2
operator!(const glm::vec3 & v)
{
	return {v.x, v.y};
}

constexpr inline glm::vec3
operator^(const glm::vec2 & v, float z)
{
	return {v.x, v.y, z};
}

constexpr inline glm::vec3
operator!(const glm::vec2 & v)
{
	return v ^ 0.F;
}

constexpr inline float
arc_length(const Arc & arc)
{
	return arc.second - arc.first;
}

float normalize(float ang);

std::pair<glm::vec2, bool> find_arc_centre(glm::vec2 start, float entrys, glm::vec2 end, float entrye);
std::pair<glm::vec2, bool> find_arc_centre(glm::vec2 start, glm::vec2 ad, glm::vec2 end, glm::vec2 bd);
std::pair<float, float> find_arcs_radius(glm::vec2 start, float entrys, glm::vec2 end, float entrye);
float find_arcs_radius(glm::vec2 start, glm::vec2 ad, glm::vec2 end, glm::vec2 bd);

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

#endif
