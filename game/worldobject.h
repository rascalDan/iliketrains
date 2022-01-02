#pragma once

#include "chronology.hpp"
#include <special_members.hpp>

class WorldObject {
public:
	WorldObject() = default;
	virtual ~WorldObject() = default;
	NO_COPY(WorldObject);
	NO_MOVE(WorldObject);

	virtual void tick(TickDuration elapsed) = 0;
};
