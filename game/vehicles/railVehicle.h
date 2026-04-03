#pragma once

#include "railVehicleClass.h"
#include <game/selectable.h>
#include <glm/glm.hpp>
#include <memory>

template<typename> class Ray;
class Train;

class RailVehicle : Selectable, RailVehicleClass::Instance, public InstanceLights {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc, GlobalPosition3D = {});

	void move(const Train *, float & trailBy);

	[[nodiscard]] Location getLocation() const;
	[[nodiscard]] bool intersectRay(const Ray<GlobalPosition3D> &, BaryPosition &, RelativeDistance &) const override;

	RailVehicleClassPtr rvClass;
};

using RailVehiclePtr = std::unique_ptr<RailVehicle>;
