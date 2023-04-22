#pragma once

#include "foliage.h"
#include "game/worldobject.h"
#include "location.hpp"
#include "maths.h"
#include <glm/gtx/transform.hpp>

class Plant : public WorldObject {
	std::shared_ptr<const Foliage> type;
	InstanceVertices<glm::mat4>::InstanceProxy location;

	void
	tick(TickDuration) override
	{
	}

public:
	Plant(std::shared_ptr<const Foliage> type, Location position);
};
