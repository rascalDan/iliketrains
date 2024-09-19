#define BOOST_TEST_MODULE environment
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <stream_support.h>

#include <config/types.h>
#include <maths.h>

// Based on the C++ code published at https://www.psa.es/sdg/sunpos.htm
// Linked from https://www.pveducation.org/pvcdrom/properties-of-sunlight/suns-position-to-high-accuracy
Direction2D
getSunPos(const Direction2D position, const float timeOfYear2024)
{
	auto & longitude = position.x;
	auto & latitude = position.y;
	using std::acos;
	using std::asin;
	using std::atan2;
	using std::cos;
	using std::floor;
	using std::sin;
	using std::tan;
	constexpr auto JD2451545 {946728000}; // which is noon 1 January 2000 Universal Time
	constexpr auto J11 {1704067200}; // 31st Dec 2023, so timeOfYear2024 1 is 1st Jan etc
	constexpr auto JDiff = static_cast<float>(J11 - JD2451545);

	// Calculate difference in days between the current Julian Day
	// and JD 2451545.0, which is noon 1 January 2000 Universal Time
	// Calculate time of the day in UT decimal hours
	const auto dDecimalHours = 24.F * (timeOfYear2024 - floor(timeOfYear2024));
	const auto dElapsedJulianDays = (JDiff + timeOfYear2024 * 86400.F) / 86400.F;

	// Calculate ecliptic coordinates (ecliptic longitude and obliquity of the
	// ecliptic in radians but without limiting the angle to be less than 2*Pi
	// (i.e., the result may be greater than 2*Pi)
	const auto dOmega = 2.1429F - 0.0010394594F * dElapsedJulianDays;
	const auto dMeanLongitude = 4.8950630F + 0.017202791698F * dElapsedJulianDays; // Radians
	const auto dMeanAnomaly = 6.2400600F + 0.0172019699F * dElapsedJulianDays;
	const auto dEclipticLongitude = dMeanLongitude + 0.03341607F * sin(dMeanAnomaly)
			+ 0.00034894F * sin(2 * dMeanAnomaly) - 0.0001134F - 0.0000203F * sin(dOmega);
	const auto dEclipticObliquity = 0.4090928F - 6.2140e-9F * dElapsedJulianDays + 0.0000396F * cos(dOmega);

	// Calculate celestial coordinates ( right ascension and declination ) in radians
	// but without limiting the angle to be less than 2*Pi (i.e., the result may be
	// greater than 2*Pi)
	const auto dSin_EclipticLongitude = sin(dEclipticLongitude);
	const auto dY = cos(dEclipticObliquity) * dSin_EclipticLongitude;
	const auto dX = cos(dEclipticLongitude);
	auto dRightAscension = atan2(dY, dX);
	if (dRightAscension < 0) {
		dRightAscension = dRightAscension + two_pi;
	}
	const auto dDeclination = asin(sin(dEclipticObliquity) * dSin_EclipticLongitude);

	// Calculate local coordinates ( azimuth and zenith angle ) in degrees
	const auto dGreenwichMeanSiderealTime = 6.6974243242F + 0.0657098283F * dElapsedJulianDays + dDecimalHours;
	const auto dLocalMeanSiderealTime
			= (dGreenwichMeanSiderealTime * 15.0F + (longitude / degreesToRads)) * degreesToRads;
	const auto dHourAngle = dLocalMeanSiderealTime - dRightAscension;
	const auto dLatitudeInRadians = latitude;
	const auto dCos_Latitude = cos(dLatitudeInRadians);
	const auto dSin_Latitude = sin(dLatitudeInRadians);
	const auto dCos_HourAngle = cos(dHourAngle);
	Direction2D udtSunCoordinates;
	udtSunCoordinates.y
			= (acos(dCos_Latitude * dCos_HourAngle * cos(dDeclination) + sin(dDeclination) * dSin_Latitude));
	udtSunCoordinates.x = atan2(-sin(dHourAngle), tan(dDeclination) * dCos_Latitude - dSin_Latitude * dCos_HourAngle);
	if (udtSunCoordinates.x < 0) {
		udtSunCoordinates.x = udtSunCoordinates.x + two_pi;
	}
	// Parallax Correction
	const auto dParallax = (earthMeanRadius / astronomicalUnit) * sin(udtSunCoordinates.y);
	udtSunCoordinates.y = half_pi - (udtSunCoordinates.y + dParallax);

	return udtSunCoordinates;
}

using sunPosTestData = std::tuple<Direction2D, float, Direction2D>;
constexpr Direction2D Doncaster = {-1.1, 53.5};
constexpr Direction2D NewYork = {74.0, 40.7};
constexpr Direction2D Syndey = {-151.2, -33.9};

BOOST_DATA_TEST_CASE(sun_position,
		boost::unit_test::data::make<sunPosTestData>({
				{{0.F, 0.F}, 1.00F, {181.52F, -66.86F}},
				{{0.F, 0.F}, 1.25F, {113.12F, -0.85F}},
				{{0.F, 0.F}, 1.50F, {177.82F, 66.97F}},
				{{0.F, 0.F}, 1.75F, {246.99F, 0.90F}},
				{{0.F, 0.F}, 2.00F, {181.52F, -67.04F}},
				{{0.F, 0.F}, 180.50F, {2.1F, 66.80F}},
				{Doncaster, 180.50F, {176.34F, 59.64F}},
				{NewYork, 180.50F, {278.04F, 27.34F}},
				{Syndey, 180.50F, {106.13F, -63.29F}},
		}),
		position, timeOfYear, expSunPos)
{
	const auto sunPos = getSunPos(position * degreesToRads, timeOfYear) / degreesToRads;
	BOOST_CHECK_CLOSE(sunPos.x, expSunPos.x, 1.F);
	BOOST_CHECK_CLOSE(sunPos.y, expSunPos.y, 1.F);
}
