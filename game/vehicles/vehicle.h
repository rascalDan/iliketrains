#ifndef VEHICLE_H
#define VEHICLE_H

#include "game/physical.h"
#include <game/network/link.h>
#include <game/worldobject.h>
#include <string>

class LinkHistory {
public:
	using Entry = std::pair<LinkWPtr, unsigned char /*dir*/>;
	using Return = std::pair<LinkCPtr, unsigned char /*dir*/>;
	Return add(const LinkWPtr &, unsigned char);
	Return getAt(float) const;

private:
	std::vector<Entry> links;
	float totalLen;
};

class Vehicle : public WorldObject, public Physical {
public:
	Vehicle(const LinkPtr & link, const std::string & obj, const std::string & tex);
	float linkDist {0}; // distance long current link
	float speed {50}; // speed in m/s (~75 km/h)

protected:
	LinkHistory linkHist;
};

#endif
