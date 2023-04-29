#include "location.h"
#include "maths.h"
#include <glm/gtx/transform.hpp>

glm::mat4
Location::getTransform() const
{
	return glm::translate(pos) * rotate_ypr(rot);
}
