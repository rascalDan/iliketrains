#include "location.h"
#include "maths.h"
#include <glm/gtx/transform.hpp>

glm::mat4
Location::getRotationTransform() const
{
	return rotate_ypr(rot);
}
