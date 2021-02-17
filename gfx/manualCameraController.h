#ifndef MANUAL_CAMERA_CONTROLLER_H
#define MANUAL_CAMERA_CONTROLLER_H

#include "game/worldobject.h"
#include "inputHandler.h"
#include <SDL2/SDL.h>
#include <gfx/camera_controller.h>
#include <optional>

class Camera;

class ManualCameraController : public CameraController, public InputHandler {
public:
	bool handleInput(SDL_Event & e) override;

	void tick(TickDuration) override { }

	void updateCamera(Camera * camera) const override;

private:
	bool ctrl {false}, mrb {false};
	mutable std::optional<SDL_MouseMotionEvent> motion;
};
#endif
