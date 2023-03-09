#include "mutation.h"
#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <maths.h>

Mutation::Matrix
Mutation::getMatrix() const
{
	return glm::translate(glm::identity<Matrix>(), position) * rotate_ypr(rotation)
			* glm::scale(glm::identity<Matrix>(), scale);
}

Mutation::Matrix
Mutation::getDeformationMatrix() const
{
	return glm::scale(glm::identity<Matrix>(), scale);
}

Mutation::Matrix
Mutation::getLocationMatrix() const
{
	return glm::translate(glm::identity<Matrix>(), position) * rotate_ypr(rotation);
}

float
Mutation::relativeLevelOfDetail() const
{
	return std::max({scale.x, scale.y, scale.z});
}
