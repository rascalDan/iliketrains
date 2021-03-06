#ifndef RAILVEHICLE_H
#define RAILVEHICLE_H

#include "gfx/renderable.h"
#include "railVehicleClass.h"
#include <array>
#include <location.hpp>
#include <memory>
#include <utility>

class Shader;

class Train;
class RailVehicle : public Renderable {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc) : rvClass {std::move(rvc)} { }

	void move(const Train *, float & trailBy);

	void render(const Shader & shader) const override;

	Location location;

	RailVehicleClassPtr rvClass;
	std::array<Location, 2> bogies;
};
using RailVehiclePtr = std::unique_ptr<RailVehicle>;

#endif
