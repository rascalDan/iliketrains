#pragma once

#include "foliage.h"
#include "game/worldobject.h"

class Location;

class Plant : public WorldObject {
	std::shared_ptr<const Foliage> type;
	InstanceVertices<Foliage::InstanceVertex>::InstanceProxy instance;

	void
	tick(TickDuration) override
	{
	}

public:
	Plant(std::shared_ptr<const Foliage> type, const Location & position);
};
