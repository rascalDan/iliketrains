#include "game/network/link.h"
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
	explicit Brush47(const LinkPtr & p);
};
