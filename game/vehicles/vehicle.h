#ifndef VEHICLE_H
#define VEHICLE_H

#include "game/physical.h"
#include <game/network/link.h>
#include <game/worldobject.h>
#include <string>

class Vehicle : public WorldObject, public Physical {
public:
	Vehicle(const LinkPtr & link, const std::string & obj, const std::string & tex);
	LinkWPtr link; // Which link are we travelling along
	unsigned char linkDir {0}; // Starting end e0->e1 or e1->e0
	float linkDist {0}; // distance long current link
	float speed {50}; // speed in m/s (~75 km/h)
};

#endif
