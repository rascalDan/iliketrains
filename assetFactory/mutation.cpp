#include "mutation.h"
#include <glm/gtx/transform.hpp>
#include <maths.h>

Mutation::Matrix
Mutation::getMatrix() const
{
	return glm::translate(glm::identity<Matrix>(), position) * rotate_ypr(rotation)
			* glm::scale(glm::identity<Matrix>(), scale);
}
