#include "manualCameraController.h"
#include <algorithm>
#include <cmath>
#include <gfx/camera.h>
#include <maths.h>

bool
ManualCameraController::handleInput(const SDL_Event & e)
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
				case SDLK_KP_8:
					direction = 0;
					break;
				case SDLK_KP_4:
					direction = -half_pi;
					break;
				case SDLK_KP_6:
					direction = half_pi;
					break;
				case SDLK_KP_2:
					direction = pi;
					break;
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
				if (ctrl) {
					direction -= 0.01F * static_cast<float>(e.motion.xrel);
					pitch = std::clamp(pitch - 0.01F * static_cast<float>(e.motion.yrel), 0.1F, half_pi);
				}
				else {
					focus += rotate_flat(-direction)
							* (RelativePosition2D {-e.motion.xrel, e.motion.yrel} * dist / 2.0F);
				}
			}
			return true;
		case SDL_MOUSEWHEEL:
			dist = std::clamp(dist - static_cast<float>(e.wheel.y) * 400.F, 5.F, 200000.F);
			break;
	}
	return false;
}

void
ManualCameraController::render(const UIShader &) const
{
}

void
ManualCameraController::updateCamera(Camera * camera) const
{
	const auto forward = glm::normalize(sincos(direction) || -sin(pitch));
	camera->setView((focus || 0) - (forward * 3.F * std::pow(dist, 1.3F)), forward);
}
