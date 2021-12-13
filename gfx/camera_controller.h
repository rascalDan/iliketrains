#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <special_members.hpp>

class Camera;

class CameraController {
public:
	CameraController() = default;
	virtual ~CameraController() = default;
	DEFAULT_MOVE_COPY(CameraController);

	virtual void updateCamera(Camera *) const = 0;
};

#endif
