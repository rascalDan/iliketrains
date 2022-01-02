#pragma once

#include <special_members.hpp>

class Camera;

class CameraController {
public:
	CameraController() = default;
	virtual ~CameraController() = default;
	DEFAULT_MOVE_COPY(CameraController);

	virtual void updateCamera(Camera *) const = 0;
};
