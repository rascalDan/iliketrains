#ifndef FOLLOW_CAMERA_CONTROLLER_H
#define FOLLOW_CAMERA_CONTROLLER_H

#include "game/worldobject.h"
#include <game/vehicles/vehicle.h>
#include <gfx/camera_controller.h>

class Camera;
class Shader;

class FollowCameraController : public CameraController {
public:
	enum class Mode { Pan, Ride, ISO };
	explicit FollowCameraController(VehicleWPtr, Mode = Mode::Pan);

	void tick(TickDuration) override { }

	void updateCamera(Camera * camera, Shader * shader) const override;

private:
	VehicleWPtr target;
	Mode mode;
};
#endif
