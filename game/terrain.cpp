#include "terrain.h"
#include "game/geoData.h"
#include "gfx/models/texture.h"
#include <algorithm>
#include <cstddef>
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

static constexpr RGB openSurface {-1};

Terrain::Terrain(std::shared_ptr<GeoData> tm) : geoData {std::move(tm)}, grass {std::make_shared<Texture>("grass.png")}
{
	generateMeshes();
}

template<>
VertexArrayObject &
VertexArrayObject::addAttribsFor<Terrain::Vertex>(const GLuint arrayBuffer, const GLuint divisor)
{
	return addAttribs<Terrain::Vertex, &Terrain::Vertex::pos, &Terrain::Vertex::normal, &Terrain::Vertex::colourBias>(
			arrayBuffer, divisor);
}

void
Terrain::generateMeshes()
{
	meshes.removeAll();
	std::vector<unsigned int> indices;
	indices.reserve(geoData->n_faces() * 3);
	std::vector<Vertex> vertices;
	vertices.reserve(geoData->n_vertices());
	std::map<std::pair<GeoData::VertexHandle, const Surface *>, size_t> vertexIndex;
	std::for_each(geoData->vertices_sbegin(), geoData->vertices_end(),
			[this, &vertexIndex, &vertices](const GeoData::VertexHandle v) {
				std::for_each(geoData->vf_begin(v), geoData->vf_end(v),
						[&vertexIndex, v, this, &vertices](const GeoData::FaceHandle f) {
							const auto surface = geoData->get_surface(f);
							if (const auto vertexIndexRef = vertexIndex.emplace(std::make_pair(v, surface), 0);
									vertexIndexRef.second) {
								vertexIndexRef.first->second = vertices.size();

								vertices.emplace_back(geoData->point(v), geoData->normal(v),
										surface ? surface->colorBias : openSurface);
							}
						});
			});
	std::for_each(
			geoData->faces_sbegin(), geoData->faces_end(), [this, &vertexIndex, &indices](const GeoData::FaceHandle f) {
				std::transform(geoData->fv_begin(f), geoData->fv_end(f), std::back_inserter(indices),
						[&vertexIndex, f, this](const GeoData::VertexHandle v) {
							return vertexIndex[std::make_pair(v, geoData->get_surface(f))];
						});
			});
	meshes.create<MeshT<Vertex>>(vertices, indices);
}

void
Terrain::tick(TickDuration)
{
}

void
Terrain::render(const SceneShader & shader) const
{
	shader.landmass.use();
	grass->bind();
	meshes.apply(&Mesh::Draw);
}

void
Terrain::shadows(const ShadowMapper & shadowMapper) const
{
	shadowMapper.landmess.use();
	meshes.apply(&Mesh::Draw);
}
