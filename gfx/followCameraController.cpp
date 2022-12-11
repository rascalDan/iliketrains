#include "followCameraController.h"
#include "game/vehicles/vehicle.h"
#include <gfx/gl/camera.h>
#include <glm/glm.hpp>
#include <location.hpp>
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
		return std::tie(t->getLocation().pos, t->getLocation().rot);
	}();

	switch (mode) {
		case Mode::Pan:
			camera->lookAt(pos);
			break;

		case Mode::Ride:
			camera->setView(pos + (up * 4.8F), !-sincosf(rot.y));
			break;

		case Mode::ISO:
			camera->setView(pos + ((up + north + east) * 40.F), glm::normalize(down + south + west),
					glm::normalize(up - north - east));
			break;
	}
}
