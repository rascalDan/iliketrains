#pragma once

#include "chronology.h"
#include "game/activities/go.h" // IWYU pragma: keep
#include "game/activities/idle.h" // IWYU pragma: keep
#include "game/activity.h"
#include "game/network/link.h"
#include "railVehicle.h"
#include "vehicle.h"
#include <collection.h>
#include <glm/glm.hpp>
#include <location.h>

class SceneShader;
class ShadowMapper;
template<typename> class Ray;

class Train : public Vehicle, public UniqueCollection<RailVehicle>, public Can<Go>, public Can<Idle> {
public:
	explicit Train(const Link::CPtr & link, float linkDist = 0) : Vehicle {link, linkDist} { }

	[[nodiscard]] const Location &
	getLocation() const override
	{
		return objects.front()->location;
	}

	[[nodiscard]] bool intersectRay(const Ray<GlobalPosition3D> &, BaryPosition &, RelativeDistance &) const override;

	void tick(TickDuration elapsed) override;
	void doActivity(Go *, TickDuration) override;
	void doActivity(Idle *, TickDuration) override;

	[[nodiscard]] Location getBogiePosition(float linkDist, float dist) const;
};
