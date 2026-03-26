#include "lightDirection.h"
#include "maths.h"

constexpr auto TWILIGHT_START = .0_degrees;
constexpr auto TWILIGHT_END = -18.0_degrees;
constexpr auto TWILIGHT_RANGE = TWILIGHT_START - TWILIGHT_END;

constexpr auto SUN_ANGLUAR_SIZE = 0.5_degrees;
constexpr auto SUN_ANGLUAR_RADIUS = SUN_ANGLUAR_SIZE / 2;
constexpr auto SUN_ELEVATION_REFRACTION_OFFSET = 0.5_degrees;
constexpr auto SUN_ANGLUAR_OFFSET = SUN_ANGLUAR_RADIUS + SUN_ELEVATION_REFRACTION_OFFSET;

constexpr auto ATMOSPHERE_SCATTER_MIN = .4F;
constexpr auto ATMOSPHERE_SCATTER_MAX = .7F;
constexpr auto ATMOSPHERE_SCATTER_RANGE = ATMOSPHERE_SCATTER_MAX - ATMOSPHERE_SCATTER_MIN;

constexpr auto NORM = 0.5F;

LightDirection::LightDirection(const Direction2D sunPos) :
	pos {sunPos}, vec {glm::mat3 {rotate_yp(pi + sunPos.x, -sunPos.y)} * north},
	amb {glm::clamp((sunPos.y - TWILIGHT_END) / TWILIGHT_RANGE, 0.F, 1.F)},
	dir {(-std::cos(std::clamp((sunPos.y + SUN_ANGLUAR_OFFSET) / SUN_ANGLUAR_SIZE, 0.F, 1.F) * pi) * NORM) + NORM},
	atmosScatter {((half_pi - sunPos.y) / half_pi * ATMOSPHERE_SCATTER_RANGE) + ATMOSPHERE_SCATTER_MIN}
{
}
