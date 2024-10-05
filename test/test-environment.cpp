#define BOOST_TEST_MODULE environment
#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <stream_support.h>

#include <chronology.h>
#include <config/types.h>
#include <game/environment.h>
#include <gfx/lightDirection.h>
#include <maths.h>

using sunPosTestData = std::tuple<Direction2D, time_t, Direction2D>;
using sunDirTestData = std::tuple<Direction2D, Direction3D, float>;
constexpr Direction2D Doncaster = {-1.1, 53.5};
constexpr Direction2D NewYork = {74.0, 40.7};
constexpr Direction2D Syndey = {-151.2, -33.9};
constexpr Direction2D EqGM = {};

BOOST_DATA_TEST_CASE(sun_position,
		boost::unit_test::data::make<sunPosTestData>({
				{EqGM, "2024-01-02T00:00:00"_time_t, {181.52F, -66.86F}},
				{EqGM, "2024-01-02T06:00:00"_time_t, {113.12F, -0.85F}},
				{EqGM, "2024-01-02T06:30:00"_time_t, {113.12F, 6.05F}},
				{EqGM, "2024-01-02T12:00:00"_time_t, {177.82F, 66.97F}},
				{EqGM, "2024-01-02T18:00:00"_time_t, {246.99F, 0.90F}},
				{EqGM, "2024-01-03T00:00:00"_time_t, {181.52F, -67.04F}},
				{EqGM, "2024-06-29T12:00:00"_time_t, {2.1F, 66.80F}},
				{Doncaster, "2024-06-29T12:00:00"_time_t, {176.34F, 59.64F}},
				{NewYork, "2024-06-29T12:00:00"_time_t, {278.04F, 27.34F}},
				{Syndey, "2024-06-29T12:00:00"_time_t, {106.13F, -63.29F}},
		}),
		position, timeOfYear, expSunPos)
{
	const auto sunPos = Environment::getSunPos(position * degreesToRads, timeOfYear) / degreesToRads;
	BOOST_CHECK_CLOSE(sunPos.x, expSunPos.x, 1.F);
	BOOST_CHECK_CLOSE(sunPos.y, expSunPos.y, 1.F);
}

BOOST_DATA_TEST_CASE(sun_direction,
		boost::unit_test::data::make<sunDirTestData>({
				{{0.F, 0.F}, south, 0.1F},
				{{90.F, 0.F}, west, 0.1F},
				{{-90.F, 0.F}, east, 0.1F},
				// From above
				// EqGM midnight, sun below horizon, shining upwards
				{{181.52F, -66.86F}, {-0.01F, 0.39F, 0.919F}, 0},
				// EqGM just before sunrise, mostly west, north a bit, up a bit
				{{113.12F, -0.85F}, {-0.92F, 0.39F, 0.015F}, 0.085F},
				// EqGM just after sunrise, mostly west, north a bit, down a bit
				{{113.12F, 6.05F}, {-0.92F, 0.39F, -0.015F}, 0.205F},
				// Doncaster noon, roughly from south to north, high in the sky, downward
				{{176.34F, 59.64F}, {-0.03F, 0.5F, -0.86F}, 0.96F},
		}),
		position, direction, vert)
{
	const LightDirection ld {position * degreesToRads};
	BOOST_CHECK_CLOSE_VEC(ld.vector(), direction);
	BOOST_CHECK_CLOSE(glm::length(ld.vector()), 1.F, 1);
	BOOST_CHECK_CLOSE(ld.vertical(), vert, 5);
}
