#pragma once

#include "gfx/gl/bufferedLocation.h"
#include "railVehicleClass.h"
#include <array>
#include <game/selectable.h>
#include <glm/glm.hpp>
#include <memory>

template<typename> class Ray;
class Train;

class RailVehicle : Selectable, RailVehicleClass::Instance {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc);

	void move(const Train *, float & trailBy);

	[[nodiscard]] bool intersectRay(const Ray<GlobalPosition3D> &, BaryPosition &, RelativeDistance &) const override;

	RailVehicleClassPtr rvClass;
	using LV = RailVehicleClass::LocationVertex;
	BufferedLocationUpdater location;
	std::array<BufferedLocationUpdater, 2> bogies;
};

using RailVehiclePtr = std::unique_ptr<RailVehicle>;
