#pragma once

#include "chronology.h"
#include <special_members.h>

class WorldObject {
public:
	WorldObject() = default;
	virtual ~WorldObject() = default;
	NO_COPY(WorldObject);
	NO_MOVE(WorldObject);

	virtual void tick(TickDuration elapsed) = 0;
};
