#pragma once

#include "uiComponent.h"
#include <SDL2/SDL.h>
#include <gfx/camera_controller.h>
#include <glm/glm.hpp>
#include <maths.h>

class Camera;

class ManualCameraController : public CameraController, public UIComponent {
public:
	explicit ManualCameraController(GlobalPosition2D f) : focus {f} { }

	bool handleInput(const SDL_Event & e) override;
	void render() override;

	void updateCamera(Camera * camera) const override;

private:
	bool ctrl {false}, mrb {false};
	GlobalPosition2D focus;
	float direction {quarter_pi};
	float dist {4000}, pitch {quarter_pi};
};
