#include "game/network/link.h"
#include "game/vehicles/vehicle.h"
#include "game/worldobject.h"
#include <memory>
#include <string>
#include <vector>

class RailVehicle : public Vehicle {
public:
	using Vehicle::Vehicle;
	float wheelBase;
	float length;
	friend class RailLoco;
};

class RailWagon : public RailVehicle {
public:
	using RailVehicle::RailVehicle;
	void tick(TickDuration elapsed) override;
};
using RailWagonPtr = std::weak_ptr<RailWagon>;

class RailLoco : public RailVehicle {
public:
	using RailVehicle::RailVehicle;
	void tick(TickDuration elapsed) override;

	std::vector<RailWagonPtr> wagons;

private:
	void move(TickDuration dur);
	Transform getBogeyPosition(float linkDist, float dist) const;
	void updateRailVehiclePosition(RailVehicle *, float trailBy) const;
	void updateWagons() const;
};

class Brush47 : public RailLoco {
public:
	explicit Brush47(const LinkPtr & p);
};

class Brush47Wagon : public RailWagon {
public:
	explicit Brush47Wagon(const LinkPtr & p);
};
