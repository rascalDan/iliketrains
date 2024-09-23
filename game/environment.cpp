#include "environment.h"
#include <chronology.h>
#include <gfx/gl/sceneRenderer.h>

Environment::Environment() : worldTime {"2024-01-01T12:00:00"_time_t} { }

void
Environment::tick(TickDuration)
{
	worldTime += 50;
}

void
Environment::render(const SceneRenderer & renderer, const SceneProvider & scene) const
{
	constexpr RGB baseAmbient {0.1F}, baseDirectional {0.0F};
	constexpr RGB relativeAmbient {0.3F, 0.3F, 0.4F}, relativeDirectional {0.6F, 0.6F, 0.5F};

	const auto sunPos = getSunPos({}, worldTime);
	const auto sunDir = (glm::mat3 {rotate_yp({sunPos.y + pi, sunPos.x})} * north);
	const auto vertical = -std::min(0.F, sunDir.z - 0.1F);
	const auto ambient = baseAmbient + relativeAmbient * vertical;
	const auto directional = baseDirectional + relativeDirectional * vertical;

	renderer.setAmbientLight(ambient);
	renderer.setDirectionalLight(directional, sunDir, scene);
}

// Based on the C++ code published at https://www.psa.es/sdg/sunpos.htm
// Linked from https://www.pveducation.org/pvcdrom/properties-of-sunlight/suns-position-to-high-accuracy
Direction2D
Environment::getSunPos(const Direction2D position, const time_t time)
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
	static const auto JD2451545 = "2000-01-01T12:00:00"_time_t;

	// Calculate difference in days between the current Julian Day
	// and JD 2451545.0, which is noon 1 January 2000 Universal Time
	// Calculate time of the day in UT decimal hours
	const auto dDecimalHours = static_cast<float>(time % 86400) / 3600.F;
	const auto dElapsedJulianDays = static_cast<float>(time - JD2451545) / 86400.F;

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
