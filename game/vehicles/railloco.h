#include "game/network/link.h"
#include "game/vehicles/vehicle.h"
#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include <array>
#include <location.hpp>
#include <memory>
#include <utility>
#include <vector>

class Shader;
class Texture;

class RailVehicleClass {
public:
	void render(const Shader &, const Location &, const std::array<Location, 2> &) const;
	std::array<MeshPtr, 2> bogies;
	MeshPtr bodyMesh;
	std::shared_ptr<Texture> texture;
	float wheelBase;
	float length;
};
using RailVehicleClassPtr = std::shared_ptr<RailVehicleClass>;

class RailVehicle : public Vehicle {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc, const LinkPtr & link, float linkDist = 0) :
		Vehicle {link, linkDist}, rvClass {std::move(rvc)}
	{
	}
	void render(const Shader & shader) const override;

	RailVehicleClassPtr rvClass;
	std::array<Location, 2> bogies;

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
	[[nodiscard]] Location getBogiePosition(float linkDist, float dist) const;
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
