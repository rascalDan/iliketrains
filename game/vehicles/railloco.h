#include "game/worldobject.h"
#include "vehicle.h"
#include <string>

class RailLoco : public Vehicle {
public:
	using Vehicle::Vehicle;
	void tick(TickDuration elapsed) override;
};

class Brush47 : public RailLoco {
public:
	Brush47(LinkPtr p);
};
