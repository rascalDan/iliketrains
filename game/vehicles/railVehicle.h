#ifndef RAILVEHICLE_H
#define RAILVEHICLE_H

#include "gfx/renderable.h"
#include "railVehicleClass.h"
#include <array>
#include <game/selectable.h>
#include <glm/glm.hpp>
#include <location.hpp>
#include <memory>
#include <utility>

class Shader;

class Train;
class RailVehicle : public Renderable, Selectable {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc) : rvClass {std::move(rvc)} { }

	void move(const Train *, float & trailBy);

	void render(const Shader & shader) const override;
	[[nodiscard]] bool intersectRay(const glm::vec3 &, const glm::vec3 &, glm::vec2 *, float *) const override;

	Location location;

	RailVehicleClassPtr rvClass;
	std::array<Location, 2> bogies;
};
using RailVehiclePtr = std::unique_ptr<RailVehicle>;

#endif
