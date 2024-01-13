#pragma once

#include "game/worldobject.h"
#include "illuminator.h"

class Location;

class Light : public WorldObject {
	std::shared_ptr<const Illuminator> type;
	InstanceVertices<Illuminator::LocationVertex>::InstanceProxy location;

	void
	tick(TickDuration) override
	{
	}

public:
	Light(std::shared_ptr<const Illuminator> type, const Location & position);
};
