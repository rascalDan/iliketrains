#define BOOST_TEST_MODULE environment
#include "testHelpers.h"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <stream_support.h>

#include <chronology.h>
#include <config/types.h>
#include <game/environment.h>
#include <gfx/lightDirection.h>
#include <maths.h>

namespace {
	using SunPosTestData = std::tuple<Direction2D, time_t, Direction2D>;
	using SunDirTestData = std::tuple<Direction2D, Direction3D>;
	using SunAmtTestData = std::tuple<float, float>;
	constexpr Direction2D DONCASTER = {-1.1, 53.5};
	constexpr Direction2D NEW_YORK = {74.0, 40.7};
	constexpr Direction2D SYNDEY = {-151.2, -33.9};
	constexpr Direction2D EQ_GM = {};
}

BOOST_DATA_TEST_CASE(SunPosition,
		boost::unit_test::data::make<SunPosTestData>({
				{EQ_GM, "2024-01-02T00:00:00"_time_t, {181.52F, -66.86F}},
				{EQ_GM, "2024-01-02T06:00:00"_time_t, {113.12F, -0.85F}},
				{EQ_GM, "2024-01-02T06:30:00"_time_t, {113.12F, 6.05F}},
				{EQ_GM, "2024-01-02T12:00:00"_time_t, {177.82F, 66.97F}},
				{EQ_GM, "2024-01-02T18:00:00"_time_t, {246.99F, 0.90F}},
				{EQ_GM, "2024-01-03T00:00:00"_time_t, {181.52F, -67.04F}},
				{EQ_GM, "2024-06-29T12:00:00"_time_t, {2.1F, 66.80F}},
				{DONCASTER, "2024-06-29T12:00:00"_time_t, {176.34F, 59.64F}},
				{NEW_YORK, "2024-06-29T12:00:00"_time_t, {278.04F, 27.34F}},
				{SYNDEY, "2024-06-29T12:00:00"_time_t, {106.13F, -63.29F}},
		}),
		position, timeOfYear, expSunPos)
{
	const auto sunPos = Environment::getSunPos(position * degreesToRads, timeOfYear) / degreesToRads;
	BOOST_CHECK_CLOSE(sunPos.x, expSunPos.x, 1.F);
	BOOST_CHECK_CLOSE(sunPos.y, expSunPos.y, 1.F);
}

BOOST_DATA_TEST_CASE(SunDirection,
		boost::unit_test::data::make<SunDirTestData>({
				{{0.F, 0.F}, south},
				{{90.F, 0.F}, west},
				{{-90.F, 0.F}, east},
				// From above
				// EqGM midnight, sun below horizon, shining upwards
				{{181.52F, -66.86F}, {-0.01F, 0.39F, 0.919F}},
				// EqGM just before sunrise, mostly west, north a bit, up a bit
				{{113.12F, -0.85F}, {-0.92F, 0.39F, 0.015F}},
				// EqGM just after sunrise, mostly west, north a bit, down a bit
				{{113.12F, 6.05F}, {-0.92F, 0.39F, -0.015F}},
				// Doncaster noon, roughly from south to north, high in the sky, downward
				{{176.34F, 59.64F}, {-0.03F, 0.5F, -0.86F}},
		}),
		position, direction)
{
	const LightDirection lightDir {position * degreesToRads};
	BOOST_CHECK_CLOSE_VEC(lightDir.vector(), direction);
	BOOST_CHECK_CLOSE(glm::length(lightDir.vector()), 1.F, 1);
}

BOOST_DATA_TEST_CASE(SunDirectionalAmount,
		boost::unit_test::data::make<SunAmtTestData>({
				{5._degrees, 1.F},
				{1._degrees, 1.F},
				{0._degrees, 1.F},
				{-0.25_degrees, 1.F},
				{-0.5_degrees, 0.5F},
				{-0.75_degrees, 0.F},
				{-1._degrees, 0.F},
				{-5._degrees, 0.F},
		}),
		elevation, amount)
{
	const LightDirection lightDir {{0, elevation}};
	BOOST_CHECK_CLOSE(lightDir.directional(), amount, 1.F);
}

BOOST_DATA_TEST_CASE(SunAmbientAmount,
		boost::unit_test::data::make<SunAmtTestData>({
				{20._degrees, 1.F},
				{1._degrees, 1.F},
				{-9._degrees, 0.5F},
				{-18._degrees, 0.F},
				{-25._degrees, 0.F},
				{-50._degrees, 0.F},
		}),
		elevation, amount)
{
	const LightDirection lightDir {{0, elevation}};
	BOOST_CHECK_CLOSE(lightDir.ambient(), amount, 1.F);
}
