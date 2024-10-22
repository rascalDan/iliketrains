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

static_assert(pow(1, 0) == 1);
static_assert(pow(1, 1) == 1);
static_assert(pow(1, 2) == 1);
static_assert(pow(2, 0) == 1);
static_assert(pow(2, 1) == 2);
static_assert(pow(2, 2) == 4);
static_assert(pow(2, 3) == 8);
static_assert(pow(3, 0) == 1);
static_assert(pow(3, 1) == 3);
static_assert(pow(3, 2) == 9);
static_assert(pow(pi, 3) == 31.006278991699219F);

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
