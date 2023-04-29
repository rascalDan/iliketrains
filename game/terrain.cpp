#include "terrain.h"
#include "game/geoData.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <array>
#include <cache.h>
#include <cstddef>
#include <filesystem>
#include <gfx/gl/sceneShader.h>
#include <gfx/gl/shadowMapper.h>
#include <gfx/image.h>
#include <gfx/models/mesh.h>
#include <gfx/models/vertex.h>
#include <glm/glm.hpp>
#include <iterator>
#include <location.h>
#include <maths.h>
#include <utility>
#include <vector>

Terrain::Terrain(std::shared_ptr<GeoData> gd) :
	geoData {std::move(gd)}, grass {Texture::cachedTexture.get("grass.png")},
	water {Texture::cachedTexture.get("water.png")}
{
	generateMeshes();
}

void
Terrain::generateMeshes()
{
	std::vector<unsigned int> indices;
	const auto isize = geoData->getSize() - glm::uvec2 {1, 1};
	indices.reserve(static_cast<std::size_t>(isize.x * isize.y) * 6);

	const auto limit = geoData->getLimit();
	//  Indices
	constexpr std::array<glm::ivec2, 6> indices_offsets {{
			{0, 0},
			{1, 0},
			{1, 1},
			{0, 0},
			{1, 1},
			{0, 1},
	}};
	for (auto y = limit.first.y; y < limit.second.y; y += 1) {
		for (auto x = limit.first.x; x < limit.second.x; x += 1) {
			std::transform(indices_offsets.begin(), indices_offsets.end(), std::back_inserter(indices),
					[this, x, y](const auto off) {
						return geoData->at(x + off.x, y + off.y);
					});
		}
	}

	const auto nodes = geoData->getNodes();
	const auto scale = geoData->getScale();
	std::vector<Vertex> vertices;
	vertices.reserve(nodes.size());
	// Positions
	for (auto y = limit.first.y; y <= limit.second.y; y += 1) {
		for (auto x = limit.first.x; x <= limit.second.x; x += 1) {
			const glm::vec2 xy {x, y};
			vertices.emplace_back((xy * scale) ^ nodes[geoData->at(x, y)].height, xy, ::up);
		}
	}
	// Normals
	const glm::uvec2 size = geoData->getSize();
	for (auto y = limit.first.y + 1; y < limit.second.y; y += 1) {
		for (auto x = limit.first.x + 1; x < limit.second.x; x += 1) {
			const auto n {geoData->at(x, y)};
			const auto a = vertices[n - 1].pos;
			const auto b = vertices[n - size.x].pos;
			const auto c = vertices[n + 1].pos;
			const auto d = vertices[n + size.x].pos;
			vertices[n].normal = -glm::normalize(glm::cross(b - d, a - c));
		}
	}
	meshes.create<Mesh>(vertices, indices);
}

void
Terrain::tick(TickDuration dur)
{
	waveCycle += dur.count();
}

void
Terrain::render(const SceneShader & shader) const
{
	shader.landmass.use();
	grass->bind();
	meshes.apply(&Mesh::Draw);

	shader.water.use(waveCycle);
	water->bind();
	meshes.apply(&Mesh::Draw);
}

void
Terrain::shadows(const ShadowMapper & shadowMapper) const
{
	shadowMapper.fixedPoint.use();
	meshes.apply(&Mesh::Draw);
}
