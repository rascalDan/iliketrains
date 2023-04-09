#include "railVehicleClass.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/models/mesh.h"
#include "gfx/models/obj.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <array>
#include <cache.h>
#include <cmath>
#include <cstddef>
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

RailVehicleClass::RailVehicleClass() { }

bool
RailVehicleClass::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(length) && STORE_MEMBER(wheelBase) && STORE_MEMBER(maxSpeed)
			&& STORE_NAME_HELPER("bogie", bogies, Asset::MeshArrayConstruct)
			&& STORE_HELPER(bodyMesh, Asset::MeshConstruct) && Asset::persist(store);
}

void
RailVehicleClass::postLoad()
{
	texture = getTexture();
}

void
RailVehicleClass::render(
		const SceneShader & shader, const Location & location, const std::array<Location, 2> & bl) const
{
	shader.basic.use(location);
	if (texture) {
		texture->bind();
	}
	bodyMesh->Draw();
	for (auto b = 0U; b < bogies.size(); ++b) {
		shader.basic.setModel(bl[b]);
		bogies[b]->Draw();
	}
}
void
RailVehicleClass::shadows(
		const ShadowMapper & shadowMapper, const Location & location, const std::array<Location, 2> & bl) const
{
	shadowMapper.dynamicPoint.use(location);
	bodyMesh->Draw();
	for (auto b = 0U; b < bogies.size(); ++b) {
		shadowMapper.dynamicPoint.setModel(bl[b]);
		bogies[b]->Draw();
	}
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
		std::set<std::pair<float, std::size_t>> vertexIds;
		for (const auto & face : object.second) {
			for (const auto & faceElement : face) {
				vertexIds.emplace(o.vertices[faceElement.x - 1].y, faceElement.x - 1);
			}
		}
		const auto offset = (vertexIds.begin()->first + vertexIds.rbegin()->first) / 2;
		for (const auto & v : vertexIds) {
			o.vertices[v.second].y -= offset;
		}
		wheelBase += std::abs(offset);
	}
	return wheelBase;
}

float
RailVehicleClass::objectLength(ObjParser & o)
{
	const auto mme = std::minmax_element(o.vertices.begin(), o.vertices.end(), [](const auto & v1, const auto & v2) {
		return v1.y < v2.y;
	});
	return mme.second->y - mme.first->y;
}
