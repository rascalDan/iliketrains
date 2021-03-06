#include "game/network/link.h"
#include "game/vehicles/vehicle.h"
#include "game/worldobject.h"
#include "gfx/models/mesh.h"
#include "gfx/renderable.h"
#include <array>
#include <collection.hpp>
#include <location.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Shader;
class Texture;
class ObjParser;

class RailVehicleClass {
public:
	explicit RailVehicleClass(const std::string & name);

	void render(const Shader &, const Location &, const std::array<Location, 2> &) const;

	std::array<MeshPtr, 2> bogies;
	MeshPtr bodyMesh;
	std::shared_ptr<Texture> texture;
	float wheelBase;
	float length;

private:
	RailVehicleClass(std::unique_ptr<ObjParser> obj, std::shared_ptr<Texture>);
	static float bogieOffset(ObjParser & o);
	static float objectLength(ObjParser & o);
};
using RailVehicleClassPtr = std::shared_ptr<RailVehicleClass>;

class Train;
class RailVehicle : public Renderable {
public:
	explicit RailVehicle(RailVehicleClassPtr rvc) : rvClass {std::move(rvc)} { }

	void move(const Train *, float & trailBy);

	void render(const Shader & shader) const override;

	Location location;

	RailVehicleClassPtr rvClass;
	std::array<Location, 2> bogies;
};
using RailVehiclePtr = std::unique_ptr<RailVehicle>;

class Train : public Vehicle, public Collection<RailVehicle, false> {
public:
	explicit Train(const LinkPtr & link, float linkDist = 0) : Vehicle {link, linkDist} { }

	[[nodiscard]] const Location &
	getLocation() const override
	{
		return objects.front()->location;
	}

	void render(const Shader & shader) const override;

	void tick(TickDuration elapsed) override;

	void move(TickDuration dur);
	[[nodiscard]] Location getBogiePosition(float linkDist, float dist) const;
};
