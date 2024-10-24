#include "lightDirection.h"
#include "maths.h"

constexpr auto ASTRONOMICAL_TWILIGHT = 18.0_degrees;
constexpr auto SUN_ANGLUAR_SIZE = 0.5_degrees;

LightDirection::LightDirection(const Direction2D sunPos) :
	pos {sunPos}, vec {glm::mat3 {rotate_yp(pi + sunPos.x, -sunPos.y)} * north},
	amb {glm::clamp(sunPos.y + ASTRONOMICAL_TWILIGHT, 0.F, 1.F)},
	dir {glm::clamp(sunPos.y + SUN_ANGLUAR_SIZE, 0.F, 1.F)}
{
}
