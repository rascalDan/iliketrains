#include "aabb.h"

AxisAlignedBoundingBox &
AxisAlignedBoundingBox::operator+=(const GlobalPosition3D & point)
{
	min = glm::min(min, point);
	max = glm::max(max, point);
	return *this;
}

AxisAlignedBoundingBox
AxisAlignedBoundingBox::operator-(const GlobalPosition3D & viewPoint) const
{
	return {min - viewPoint, max - viewPoint};
}
