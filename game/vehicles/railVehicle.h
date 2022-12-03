#pragma once

#include "gfx/renderable.h"
#include "railVehicleClass.h"
#include <array>
#include <game/selectable.h>
#include <glm/glm.hpp>
#include <location.hpp>
#include <memory>
#include <utility>

class SceneShader;
class ShadowMapper;
class Ray;

class Train;
class RailVehicle : public Renderable, Selectable {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc) : rvClass {std::move(rvc)} { }

	void move(const Train *, float & trailBy);

	void render(const SceneShader & shader) const override;
	void shadows(const ShadowMapper & shadowMapper) const override;
	[[nodiscard]] bool intersectRay(const Ray &, glm::vec2 *, float *) const override;

	Location location;

	RailVehicleClassPtr rvClass;
	std::array<Location, 2> bogies;
};
using RailVehiclePtr = std::unique_ptr<RailVehicle>;
