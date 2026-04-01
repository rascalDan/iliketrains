#pragma once

#include "game/worldobject.h"
#include "illuminator.h"

class Location;

class Light : public WorldObject {
	std::shared_ptr<const Illuminator> type;
	InstanceVertices<Illuminator::InstanceVertex>::InstanceProxy instance;

	void
	tick(TickDuration) override
	{
	}

	std::vector<InstanceVertices<SpotLightVertex>::InstanceProxy> spotLightInstances;
	std::vector<InstanceVertices<PointLightVertex>::InstanceProxy> pointLightInstances;

public:
	Light(std::shared_ptr<const Illuminator> type, const Location & position);
};
