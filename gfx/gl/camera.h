#ifndef CAMERA_INCLUDED_H
#define CAMERA_INCLUDED_H

#include <glm/glm.hpp>

class Camera {
public:
	Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar);

	[[nodiscard]] glm::mat4 GetViewProjection() const;

	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;

private:
	glm::mat4 projection;
};

#endif
