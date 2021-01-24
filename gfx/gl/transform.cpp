#include "transform.h"
#include <glm/gtx/transform.hpp>

Transform::Transform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) : pos {pos}, rot {rot}, scale {scale} { }

glm::mat4
Transform::GetModel() const
{
	const auto posMat = glm::translate(pos);
	const auto scaleMat = glm::scale(scale);
	const auto rotX = glm::rotate(rot.x, glm::vec3(1.0, 0.0, 0.0));
	const auto rotY = glm::rotate(rot.y, glm::vec3(0.0, 1.0, 0.0));
	const auto rotZ = glm::rotate(rot.z, glm::vec3(0.0, 0.0, 1.0));
	const auto rotMat = rotX * rotY * rotZ;

	return posMat * rotMat * scaleMat;
}
