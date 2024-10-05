#include "lightDirection.h"
#include "maths.h"

LightDirection::LightDirection(const Direction2D sunPos) :
	pos {sunPos}, vec {glm::mat3 {rotate_yp(pi + sunPos.x, -sunPos.y)} * north},
	vert {-glm::clamp(-1.F, 0.F, vec.z - 0.1F)}
{
}
