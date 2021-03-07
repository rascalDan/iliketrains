#include "railVehicleClass.h"
#include "gfx/gl/shader.h"
#include "gfx/models/mesh.h"
#include "gfx/models/obj.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <array>
#include <cache.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <iterator>
#include <lib/resource.h>
#include <location.hpp>
#include <map>
#include <maths.h>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

RailVehicleClass::RailVehicleClass(const std::string & name) :
	RailVehicleClass {std::make_unique<ObjParser>(Resource::mapPath(name + ".obj")),
			Texture::cachedTexture.get(Resource::mapPath(name + ".png"))}
{
}

RailVehicleClass::RailVehicleClass(std::unique_ptr<ObjParser> o, std::shared_ptr<Texture> t) :
	texture {std::move(t)}, maxSpeed(95._mph)
{
	wheelBase = bogieOffset(*o);
	length = objectLength(*o);
	const auto m = o->createMeshes();
	bodyMesh = m.at("Body");
	bogies[0] = m.at("Bogie1");
	bogies[1] = m.at("Bogie2");
}

void
RailVehicleClass::render(const Shader & shader, const Location & location, const std::array<Location, 2> & bl) const
{
	texture->Bind();
	for (auto b = 0U; b < bogies.size(); ++b) {
		shader.setModel(bl[b]);
		bogies[b]->Draw();
	}
	shader.setModel(location);
	bodyMesh->Draw();
}

float
RailVehicleClass::bogieOffset(ObjParser & o)
{
	float wheelBase {0};
	// offset bogie positions so they can be set directly
	for (auto & object : o.objects) { // bogie object index
		if (!object.first.starts_with("Bogie")) {
			continue;
		}
		std::set<std::pair<float, int>> vertexIds;
		for (const auto & face : object.second) {
			for (const auto & faceElement : face) {
				vertexIds.emplace(o.vertices[faceElement.x - 1].z, faceElement.x - 1);
			}
		}
		const auto offset = (vertexIds.begin()->first + vertexIds.rbegin()->first) / 2;
		for (const auto & v : vertexIds) {
			o.vertices[v.second].z -= offset;
		}
		wheelBase += std::abs(offset);
	}
	return wheelBase;
}

float
RailVehicleClass::objectLength(ObjParser & o)
{
	const auto mme = std::minmax_element(o.vertices.begin(), o.vertices.end(), [](const auto & v1, const auto & v2) {
		return v1.z < v2.z;
	});
	return mme.second->z - mme.first->z;
}
