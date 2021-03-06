#ifndef VEHICLE_H
#define VEHICLE_H

#include "linkHistory.h"
#include <game/network/link.h>
#include <game/worldobject.h>
#include <gfx/renderable.h>
#include <memory>

class Location;

class Vehicle : public WorldObject, public Renderable {
public:
	explicit Vehicle(const LinkPtr & link, float linkDist = 0);
	float linkDist; // distance along current link
	float speed {50}; // speed in m/s (~75 km/h)

	[[nodiscard]] virtual const Location & getLocation() const = 0;

protected:
	LinkHistory linkHist;
};
using VehicleWPtr = std::weak_ptr<Vehicle>;

#endif
