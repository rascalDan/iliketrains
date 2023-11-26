#pragma once

#include "chronology.h"
#include "linkHistory.h"
#include <game/activity.h>
#include <game/network/link.h>
#include <game/orders.h>
#include <game/selectable.h>
#include <game/worldobject.h>
#include <gfx/renderable.h>
#include <memory>

class Location;

class Vehicle : public WorldObject, public Selectable {
public:
	explicit Vehicle(const Link::Ptr & link, float linkDist = 0);
	float linkDist; // distance along current link
	float speed {}; // speed in m/s (~75 km/h)

	[[nodiscard]] virtual const Location & getLocation() const = 0;
	Orders orders;

	ActivityPtr currentActivity;

protected:
	void move(TickDuration dur);
	LinkHistory linkHist;
};

using VehicleWPtr = std::weak_ptr<Vehicle>;
