#ifndef MATH_H
#define MATH_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <utility>

using Arc = std::pair<float, float>;

constexpr const glm::vec3 up {0, 1, 0};
constexpr const glm::vec3 north {0, 0, 1};
constexpr auto half_pi {glm::half_pi<float>()};
constexpr auto pi {glm::pi<float>()};
constexpr auto two_pi {glm::two_pi<float>()};

glm::mat4 flat_orientation(const glm::vec3 & diff);

float flat_angle(const glm::vec3 & diff);

float round_frac(const float & v, const float & frac);

constexpr inline glm::vec3
operator!(const glm::vec2 & v)
{
	return {v.x, 0, v.y};
}

constexpr inline float
arc_length(const Arc & arc)
{
	return arc.second - arc.first;
}

float normalize(float ang);

Arc create_arc(const glm::vec3 & centre3, const glm::vec3 & e0p, const glm::vec3 & e1p);

#endif