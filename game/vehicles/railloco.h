#include "game/network/link.h"
#include "game/vehicles/vehicle.h"
#include "game/worldobject.h"
#include <string>

class RailVehicle : public Vehicle {
public:
	using Vehicle::Vehicle;
	float wheelBase;
};

class RailLoco : public RailVehicle {
public:
	using RailVehicle::RailVehicle;
	void tick(TickDuration elapsed) override;
};

class Brush47 : public RailLoco {
public:
	explicit Brush47(const LinkPtr & p);
};
