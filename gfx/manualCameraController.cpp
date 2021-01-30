#include "manualCameraController.h"
#include <gfx/gl/camera.h>
#include <gfx/gl/shader.h>

bool
ManualCameraController::handleInput(SDL_Event & e)
{
	switch (e.type) {
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					ctrl = true;
					return true;
			}
			break;
		case SDL_KEYUP:
			switch (e.key.keysym.sym) {
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					ctrl = false;
					return true;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (e.button.button) {
				case SDL_BUTTON_RIGHT:
					SDL_SetRelativeMouseMode(SDL_TRUE);
					mrb = true;
					return true;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (e.button.button) {
				case SDL_BUTTON_RIGHT:
					SDL_SetRelativeMouseMode(SDL_FALSE);
					mrb = false;
					return true;
			}
			break;
		case SDL_MOUSEMOTION:
			if (mrb) {
				motion = e.motion;
			}
			return true;
	}
	return false;
}

void
ManualCameraController::updateCamera(Camera * camera, Shader * shader) const
{
	if (motion) {
		if (ctrl) {
			camera->RotateY(-0.01F * motion->xrel);
			camera->Pitch(-0.01F * motion->yrel);
		}
		else {
			camera->MoveRight(motion->xrel);
			camera->SlideForward(motion->yrel);
		}
		shader->setView(camera->GetViewProjection());
		motion.reset();
	}
}
