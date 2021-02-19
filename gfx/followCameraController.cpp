#include "followCameraController.h"
#include "game/vehicles/vehicle.h"
#include "gfx/gl/transform.h"
#include <gfx/gl/camera.h>
#include <gfx/gl/shader.h>
#include <glm/glm.hpp>
#include <maths.h>
#include <memory>
#include <tuple>
#include <utility>

FollowCameraController::FollowCameraController(VehicleWPtr t, Mode m) : target(std::move(t)), mode(m) { }

void
FollowCameraController::updateCamera(Camera * camera) const
{
	const auto [pos, rot] = [this]() {
		const auto t {target.lock()};
		return std::tie(t->location.GetPos(), t->location.GetRot());
	}();

	switch (mode) {
		case Mode::Pan:
			camera->forward = glm::normalize(pos - camera->pos);
			camera->up = up;
			break;

		case Mode::Ride:
			camera->pos = pos + (up * 4.8F);
			camera->forward = {-std::sin(rot.y), 0.F, -std::cos(rot.y)};
			camera->up = up;
			break;

		case Mode::ISO:
			camera->pos = pos + ((up + north + east) * 20.F);
			camera->forward = -glm::normalize(up + north + east);
			camera->up = glm::normalize(up - north - east);
			break;
	}
}
