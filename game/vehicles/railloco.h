#include "game/network/link.h"
#include "game/vehicles/vehicle.h"
#include "game/worldobject.h"
#include "gfx/gl/transform.h"
#include "gfx/models/mesh.h"
#include <array>
#include <memory>
#include <vector>

class Shader;

class Texture;
class RailVehicle : public Vehicle {
public:
	struct Bogie {
		Transform location;
		MeshPtr mesh;
	};

	using Vehicle::Vehicle;
	void render(const Shader & shader) const override;

	std::array<Bogie, 2> bogies;
	MeshPtr bodyMesh;
	std::shared_ptr<Texture> texture;
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
	[[nodiscard]] Transform getBogiePosition(float linkDist, float dist) const;
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
