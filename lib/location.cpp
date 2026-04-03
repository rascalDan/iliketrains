#include "location.h"
#include "maths.h"
#include <glm/gtx/transform.hpp>

Location
Location::operator+(RelativePosition3D offset) const
{
	Location ret {*this};
	ret.pos += offset;
	return ret;
}

glm::mat3
Location::getRotationTransform() const
{
	return rotate_ypr(rot);
}
