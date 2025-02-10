#include "terrain.h"
#include "game/geoData.h"
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

static constexpr RGB OPEN_SURFACE {-1};

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
	indices.reserve(n_faces() * 3);
	std::vector<Vertex> vertices;
	vertices.reserve(n_vertices());
	std::map<std::pair<VertexHandle, const Surface *>, size_t> vertexIndex;
	std::ranges::for_each(this->vertices(), [this, &vertexIndex, &vertices](const auto vertex) {
		std::ranges::for_each(vf_range(vertex), [&vertexIndex, vertex, this, &vertices](const auto face) {
			const auto * const surface = getSurface(face);
			if (const auto vertexIndexRef = vertexIndex.emplace(std::make_pair(vertex, surface), 0);
					vertexIndexRef.second) {
				vertexIndexRef.first->second = vertices.size();

				vertices.emplace_back(point(vertex), normal(vertex), surface ? surface->colorBias : OPEN_SURFACE);
			}
		});
	});
	std::ranges::for_each(faces(), [this, &vertexIndex, &indices](const auto face) {
		std::ranges::transform(
				fv_range(face), std::back_inserter(indices), [&vertexIndex, face, this](const auto vertex) {
					return vertexIndex[std::make_pair(vertex, getSurface(face))];
				});
	});
	meshes.create<MeshT<Vertex>>(vertices, indices);
}

void
Terrain::tick(TickDuration)
{
}

void
Terrain::afterChange()
{
		generateMeshes();
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
