#pragma once

#include "gfx/models/mesh.h"
#include <array>
#include <memory>
#include <string>

class SceneShader;
class Texture;
class ObjParser;
class Location;

class RailVehicleClass {
public:
	explicit RailVehicleClass(const std::string & name);

	void render(const SceneShader &, const Location &, const std::array<Location, 2> &) const;

	std::array<Mesh::Ptr, 2> bogies;
	Mesh::Ptr bodyMesh;
	std::shared_ptr<Texture> texture;
	float wheelBase;
	float length;
	float maxSpeed;

private:
	RailVehicleClass(std::unique_ptr<ObjParser> obj, std::shared_ptr<Texture>);
	static float bogieOffset(ObjParser & o);
	static float objectLength(ObjParser & o);
};
using RailVehicleClassPtr = std::shared_ptr<RailVehicleClass>;
