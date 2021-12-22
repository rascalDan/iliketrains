#include "manualCameraController.h"
#include <algorithm>
#include <cmath>
#include <gfx/gl/camera.h>
#include <maths.h>

bool
ManualCameraController::handleInput(const SDL_Event & e, const Position &)
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
				if (ctrl) {
					direction -= 0.01F * static_cast<float>(e.motion.xrel);
					pitch = std::clamp(pitch - 0.01F * static_cast<float>(e.motion.yrel), 0.1F, half_pi);
				}
				else {
					focus += rotate_flat(-direction) * glm::vec2 {-e.motion.xrel, e.motion.yrel};
				}
			}
			return true;
		case SDL_MOUSEWHEEL:
			dist = std::clamp(dist - static_cast<float>(e.wheel.y) * 4.F, 5.F, 200.F);
			break;
	}
	return false;
}

void
ManualCameraController::render(const UIShader &, const Position &) const
{
}

void
ManualCameraController::updateCamera(Camera * camera) const
{
	camera->forward = glm::normalize(sincosf(direction) ^ -sin(pitch));
	camera->pos = !focus + up * 3.F - (camera->forward * std::pow(dist, 1.3F));
	camera->up = up;
}
