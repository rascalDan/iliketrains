#ifndef VEHICLE_H
#define VEHICLE_H

#include "game/physical.h"
#include <game/network/link.h>
#include <game/worldobject.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class LinkHistory {
public:
	using WEntry = std::pair<LinkWPtr, unsigned char /*dir*/>;
	using Entry = std::pair<LinkCPtr, unsigned char /*dir*/>;
	Entry add(const LinkWPtr &, unsigned char);
	[[nodiscard]] Entry getCurrent() const;
	[[nodiscard]] Entry getAt(float, float *) const;

private:
	std::vector<WEntry> links;
	float totalLen {0.F};
};

class Vehicle : public WorldObject, public Physical {
public:
	Vehicle(const LinkPtr & link, const std::string & obj, const std::string & tex);
	float linkDist {0}; // distance long current link
	float speed {50}; // speed in m/s (~75 km/h)

protected:
	LinkHistory linkHist;
};
using VehicleWPtr = std::weak_ptr<Vehicle>;

#endif
