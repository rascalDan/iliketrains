#ifndef WORLDOBJECT_H
#define WORLDOBJECT_H

#include <chrono>
#include <special_members.hpp>

class WorldObject {
public:
	using TickDuration = std::chrono::milliseconds;

	WorldObject() = default;
	virtual ~WorldObject() = default;
	NO_COPY(WorldObject);
	NO_MOVE(WorldObject);

	virtual void tick(TickDuration elapsed) = 0;
};

#endif
