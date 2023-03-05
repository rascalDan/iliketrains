#pragma once

#include "assetFactory/asset.h"
#include "gfx/models/mesh.h"
#include <array>
#include <memory>
#include <string>

class SceneShader;
class ShadowMapper;
class Texture;
class ObjParser;
class Location;

class RailVehicleClass : public Asset {
public:
	explicit RailVehicleClass(const std::string & name);
	RailVehicleClass();

	void render(const SceneShader &, const Location &, const std::array<Location, 2> &) const;
	void shadows(const ShadowMapper &, const Location &, const std::array<Location, 2> &) const;

	std::array<Mesh::Ptr, 2> bogies;
	Mesh::Ptr bodyMesh;
	std::shared_ptr<Texture> texture;
	float wheelBase;
	float length;
	float maxSpeed;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<RailVehicleClass>>;
	bool persist(Persistence::PersistenceStore & store) override;

private:
	RailVehicleClass(std::unique_ptr<ObjParser> obj, std::shared_ptr<Texture>);
	static float bogieOffset(ObjParser & o);
	static float objectLength(ObjParser & o);
};
using RailVehicleClassPtr = std::shared_ptr<RailVehicleClass>;
