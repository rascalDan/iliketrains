#pragma once

#include <game/vehicles/vehicle.h>
#include <gfx/camera_controller.h>

class Camera;

class FollowCameraController : public CameraController {
public:
	enum class Mode { Pan, Ride, ISO };
	explicit FollowCameraController(VehicleWPtr, Mode = Mode::Pan);

	void updateCamera(Camera * camera) const override;

private:
	VehicleWPtr target;
	Mode mode;
};
