#include "terrain.h"
#include "game/geoData.h"
#include "gfx/models/texture.h"
#include <algorithm>
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

Terrain::Terrain(std::shared_ptr<GeoData> tm) :
	geoData {std::move(tm)}, grass {std::make_shared<Texture>("grass.png")},
	water {std::make_shared<Texture>("water.png")}
{
	generateMeshes();
}

void
Terrain::generateMeshes()
{
	std::vector<unsigned int> indices;
	indices.reserve(geoData->n_faces() * 3);
	std::vector<Vertex> vertices;
	vertices.reserve(geoData->n_vertices());
	std::map<GeoData::VertexHandle, size_t> vertexIndex;
	std::transform(geoData->vertices_begin(), geoData->vertices_end(), std::back_inserter(vertices),
			[this, &vertexIndex](const GeoData::VertexHandle v) {
				vertexIndex.emplace(v, vertexIndex.size());
				const auto p = geoData->point(v);
				return Vertex {p, p / 10000, geoData->normal(v)};
			});
	std::for_each(
			geoData->faces_begin(), geoData->faces_end(), [this, &vertexIndex, &indices](const GeoData::FaceHandle f) {
				std::transform(geoData->fv_begin(f), geoData->fv_end(f), std::back_inserter(indices),
						[&vertexIndex](const GeoData::VertexHandle v) {
							return vertexIndex[v];
						});
			});
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
