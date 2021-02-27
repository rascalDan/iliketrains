#ifndef MATH_H
#define MATH_H

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

constexpr const glm::vec3 up {0, 1, 0};
constexpr const glm::vec3 north {0, 0, 1};
constexpr const glm::vec3 south {0, 0, -1};
constexpr const glm::vec3 east {-1, 0, 0};
constexpr const glm::vec3 west {1, 0, 0};
constexpr auto half_pi {glm::half_pi<float>()};
constexpr auto pi {glm::pi<float>()};
constexpr auto two_pi {glm::two_pi<float>()};

glm::mat4 flat_orientation(const glm::vec3 & diff);

float vector_yaw(const glm::vec3 & diff);
float vector_pitch(const glm::vec3 & diff);

float round_frac(const float & v, const float & frac);

constexpr inline glm::vec2
operator!(const glm::vec3 & v)
{
	return {v.x, v.z};
}

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

std::pair<glm::vec2, bool> find_arc_centre(glm::vec2 start, float entrys, glm::vec2 end, float entrye);
std::pair<glm::vec2, bool> find_arc_centre(glm::vec2 start, glm::vec2 ad, glm::vec2 end, glm::vec2 bd);

#endif
