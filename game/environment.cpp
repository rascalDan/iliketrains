#include "environment.h"
#include "gfx/lightDirection.h"
#include <chronology.h>
#include <gfx/gl/sceneRenderer.h>

constexpr Direction2D DONCASTER = {-1.1_degrees, 53.5_degrees};

Environment::Environment() : worldTime {"2026-06-01T12:00:00"_seconds}, gameTimeScaleFactor {1440}, earthPos {DONCASTER}
{
}

void
Environment::tick(TickDuration elapsed)
{
	worldTime += std::chrono::duration_cast<WorldTime::duration>(elapsed * gameTimeScaleFactor);
}

Environment::WorldTime
Environment::getWorldTime() const
{
	return worldTime;
}

Direction2D
Environment::getSunPos() const
{
	return getSunPos(earthPos, worldTime.time_since_epoch().count());
}

void
Environment::render(const SceneRenderer & renderer, const SceneProvider & scene) const
{
	constexpr RGB SUN_LIGHT {1, 1, .878F};
	constexpr RGB SKY_BLUE {.529F, .808F, .922F};
	constexpr RGB BASE_AMBIENT_LIGHT {0.1F};

	const LightDirection sunPos {getSunPos()};
	const auto scattered = SKY_BLUE * sunPos.atmosphericScattering() * sunPos.ambient();
	const auto ambient = BASE_AMBIENT_LIGHT + scattered;
	const auto directional = (SUN_LIGHT - BASE_AMBIENT_LIGHT - scattered) * sunPos.directional();

	renderer.setAmbientLight(ambient);
	renderer.setDirectionalLight(directional, sunPos, scene);
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
	static const auto jD2451545 = "2000-01-01T12:00:00"_time_t;

	// Calculate difference in days between the current Julian Day
	// and JD 2451545.0, which is noon 1 January 2000 Universal Time
	// Calculate time of the day in UT decimal hours
	const auto dDecimalHours = static_cast<float>(time % 86400) / 3600.F;
	const auto dElapsedJulianDays = static_cast<float>(time - jD2451545) / 86400.F;

	// Calculate ecliptic coordinates (ecliptic longitude and obliquity of the
	// ecliptic in radians but without limiting the angle to be less than 2*Pi
	// (i.e., the result may be greater than 2*Pi)
	const auto dOmega = 2.1429F - (0.0010394594F * dElapsedJulianDays);
	const auto dMeanLongitude = 4.8950630F + (0.017202791698F * dElapsedJulianDays); // Radians
	const auto dMeanAnomaly = 6.2400600F + (0.0172019699F * dElapsedJulianDays);
	const auto dEclipticLongitude = dMeanLongitude + (0.03341607F * sin(dMeanAnomaly))
			+ (0.00034894F * sin(2 * dMeanAnomaly)) - 0.0001134F - (0.0000203F * sin(dOmega));
	const auto dEclipticObliquity = 0.4090928F - (6.2140e-9F * dElapsedJulianDays) + (0.0000396F * cos(dOmega));

	// Calculate celestial coordinates ( right ascension and declination ) in radians
	// but without limiting the angle to be less than 2*Pi (i.e., the result may be
	// greater than 2*Pi)
	const auto dSinEclipticLongitude = sin(dEclipticLongitude);
	const auto decY = cos(dEclipticObliquity) * dSinEclipticLongitude;
	const auto decX = cos(dEclipticLongitude);
	auto dRightAscension = atan2(decY, decX);
	if (dRightAscension < 0) {
		dRightAscension = dRightAscension + two_pi;
	}
	const auto dDeclination = asin(sin(dEclipticObliquity) * dSinEclipticLongitude);

	// Calculate local coordinates ( azimuth and zenith angle ) in degrees
	const auto dGreenwichMeanSiderealTime = 6.6974243242F + (0.0657098283F * dElapsedJulianDays) + dDecimalHours;
	const auto dLocalMeanSiderealTime
			= ((dGreenwichMeanSiderealTime * 15.0F) + (longitude / degreesToRads)) * degreesToRads;
	const auto dHourAngle = dLocalMeanSiderealTime - dRightAscension;
	const auto dLatitudeInRadians = latitude;
	const auto dCosLatitude = cos(dLatitudeInRadians);
	const auto dSinLatitude = sin(dLatitudeInRadians);
	const auto dCosHourAngle = cos(dHourAngle);
	Direction2D udtSunCoordinates;
	udtSunCoordinates.y
			= (acos((dCosLatitude * dCosHourAngle * cos(dDeclination)) + (sin(dDeclination) * dSinLatitude)));
	udtSunCoordinates.x = atan2(-sin(dHourAngle), (tan(dDeclination) * dCosLatitude) - (dSinLatitude * dCosHourAngle));
	if (udtSunCoordinates.x < 0) {
		udtSunCoordinates.x = udtSunCoordinates.x + two_pi;
	}
	// Parallax Correction
	const auto dParallax = (earthMeanRadius / astronomicalUnit) * sin(udtSunCoordinates.y);
	udtSunCoordinates.y = half_pi - (udtSunCoordinates.y + dParallax);

	return udtSunCoordinates;
}
