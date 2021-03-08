#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include <chrono>
#include <special_members.hpp>

using TickDuration = std::chrono::duration<float, std::chrono::seconds::period>;

class WorldObject {
public:
	WorldObject() = default;
	virtual ~WorldObject() = default;
	NO_COPY(WorldObject);
	NO_MOVE(WorldObject);

	virtual void tick(TickDuration elapsed) = 0;
};

#endif
