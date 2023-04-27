#pragma once

#include "gfx/gl/bufferedLocation.h"
#include "railVehicleClass.h"
#include <array>
#include <game/selectable.h>
#include <glm/glm.hpp>
#include <memory>
#include <utility>

class SceneShader;
class ShadowMapper;
class Ray;

class Train;
class RailVehicle : Selectable {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc);

	void move(const Train *, float & trailBy);

	[[nodiscard]] bool intersectRay(const Ray &, glm::vec2 *, float *) const override;

	RailVehicleClassPtr rvClass;
	BufferedLocation location;
	std::array<BufferedLocation, 2> bogies;
};
using RailVehiclePtr = std::unique_ptr<RailVehicle>;
