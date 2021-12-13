#ifndef MANUAL_CAMERA_CONTROLLER_H
#define MANUAL_CAMERA_CONTROLLER_H

#include "inputHandler.h"
#include <SDL2/SDL.h>
#include <gfx/camera_controller.h>
#include <glm/glm.hpp>
#include <maths.h>

class Camera;

class ManualCameraController : public CameraController, public InputHandler {
public:
	explicit ManualCameraController(glm::vec2 f) : focus {f} { }

	bool handleInput(const SDL_Event & e) override;

	void updateCamera(Camera * camera) const override;

private:
	bool ctrl {false}, mrb {false};
	glm::vec2 focus;
	float direction {quarter_pi};
	float dist {40}, pitch {quarter_pi};
};
#endif
