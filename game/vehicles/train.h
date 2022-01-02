#pragma once

#include "chronology.hpp"
#include "game/activities/go.h" // IWYU pragma: keep
#include "game/activities/idle.h" // IWYU pragma: keep
#include "game/activity.h"
#include "game/network/link.h"
#include "railVehicle.h"
#include "vehicle.h"
#include <collection.hpp>
#include <glm/glm.hpp>
#include <location.hpp>
#include <memory>
#include <vector>

class Shader;

class Train : public Vehicle, public Collection<RailVehicle, false>, public Can<Go>, public Can<Idle> {
public:
	explicit Train(const LinkPtr & link, float linkDist = 0) : Vehicle {link, linkDist} { }

	[[nodiscard]] const Location &
	getLocation() const override
	{
		return objects.front()->location;
	}

	void render(const Shader & shader) const override;

	[[nodiscard]] bool intersectRay(const glm::vec3 &, const glm::vec3 &, glm::vec2 *, float *) const override;

	void tick(TickDuration elapsed) override;
	void doActivity(Go *, TickDuration) override;
	void doActivity(Idle *, TickDuration) override;

	[[nodiscard]] Location getBogiePosition(float linkDist, float dist) const;
};
