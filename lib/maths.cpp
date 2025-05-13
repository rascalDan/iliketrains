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

// NOLINTBEGIN(readability-magic-numbers)
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

static_assert(!linesIntersectAt<int>({0, 10}, {10, 10}, {10, 0}, {0, 0}).has_value());
static_assert(*linesIntersectAt<int>({0, 0}, {10, 10}, {10, 0}, {0, 10}) == GlobalPosition2D {5, 5});
static_assert(*linesIntersectAt<int>({300'000'000, 400'000'00}, {300'010'000, 400'010'00}, {310'010'000, 410'000'00},
					  {310'000'000, 410'010'00})
		== GlobalPosition2D {310'005'000, 410'005'00});

constexpr auto NORTH2D = RelativePosition2D(north);
constexpr auto EAST2D = RelativePosition2D(east);
static_assert(!linesIntersectAtDirs<int>({0, 0}, NORTH2D, {10, 10}, NORTH2D).has_value());
static_assert(linesIntersectAtDirs<int>({0, 0}, NORTH2D, {10, 10}, EAST2D) == GlobalPosition2D {0, 10});
static_assert(linesIntersectAtDirs<int>({0, 0}, EAST2D, {10, 10}, NORTH2D) == GlobalPosition2D {10, 0});
// NOLINTEND(readability-magic-numbers)

float
operator""_mph(const long double v)
{
	return static_cast<float>(mph_to_ms(v));
}

float
operator""_kph(const long double v)
{
	return static_cast<float>(kph_to_ms(v));
}
