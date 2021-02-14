#include "transform.h"
#include <glm/gtx/transform.hpp>
#include <maths.h>

Transform::Transform(glm::vec3 pos, glm::vec3 rot) : pos {pos}, rot {rot} { }

glm::mat4
Transform::GetModel() const
{
	const auto posMat = glm::translate(pos);
	const auto rotX = glm::rotate(rot.x, west);
	const auto rotY = glm::rotate(rot.y, up);
	const auto rotZ = glm::rotate(rot.z, north);

	return posMat * rotY * rotX * rotZ;
}
