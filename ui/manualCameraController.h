#pragma once

#include "uiComponent.h"
#include <SDL2/SDL.h>
#include <gfx/camera_controller.h>
#include <glm/glm.hpp>
#include <maths.h>

class UIShader;
class Camera;

class ManualCameraController : public CameraController, public UIComponent {
public:
	explicit ManualCameraController(GlobalPosition2D f) : UIComponent {{}}, focus {f} { }

	bool handleInput(const SDL_Event & e, const Position &) override;
	void render(const UIShader &, const Position & parentPos) const override;

	void updateCamera(Camera * camera) const override;

private:
	bool ctrl {false}, mrb {false};
	GlobalPosition2D focus;
	float direction {quarter_pi};
	float dist {4000}, pitch {quarter_pi};
};
