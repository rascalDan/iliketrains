#ifndef CAMERA_INCLUDED_H
#define CAMERA_INCLUDED_H

#include <glm/glm.hpp>

class Camera {
public:
	Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar);

	[[nodiscard]] glm::mat4 GetViewProjection() const;

	void MoveForward(float amt);
	void MoveRight(float amt);
	void Pitch(float angle);
	void RotateY(float angle);

private:
	glm::mat4 projection;
	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;
};

#endif
