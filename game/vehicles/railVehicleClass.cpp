#include "railVehicleClass.h"
#include "gfx/gl/sceneShader.h"
#include "gfx/gl/shadowMapper.h"
#include "gfx/models/mesh.h"
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
