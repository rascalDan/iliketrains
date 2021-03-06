#ifndef RAILVEHICLECLASS_H
#define RAILVEHICLECLASS_H

#include "gfx/models/mesh.h"
#include <array>
#include <memory>
#include <string>

class Shader;
class Texture;
class ObjParser;
class Location;

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

#endif
