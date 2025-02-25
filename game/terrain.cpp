#include "terrain.h"
#include <algorithm>
#include <gfx/gl/sceneShader.h>
#include <gfx/gl/shadowMapper.h>
#include <gfx/image.h>
#include <gfx/models/mesh.h>
#include <gfx/models/vertex.h>
#include <glMappedBufferWriter.h>
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
	return addAttribs<Terrain::Vertex, &Terrain::Vertex::pos, &Terrain::Vertex::normal>(arrayBuffer, divisor);
}

void
Terrain::generateMeshes()
{
	std::ranges::transform(all_vertices(), glMappedBufferWriter<Vertex> {GL_ARRAY_BUFFER, verticesBuffer, n_vertices()},
			[this](const auto & vertex) {
				return Vertex {point(vertex), normal(vertex)};
			});

	std::map<const Surface *, std::vector<GLuint>> surfaceIndices;
	for (const auto face : faces()) {
		const auto * const surface = getSurface(face);
		auto indexItr = surfaceIndices.find(surface);
		if (indexItr == surfaceIndices.end()) {
			indexItr = surfaceIndices.emplace(surface, std::vector<GLuint> {}).first;
			if (!surface) {
				indexItr->second.reserve(n_vertices() * 3);
			}
		}
		std::ranges::transform(fv_range(face), std::back_inserter(indexItr->second), &OpenMesh::VertexHandle::idx);
	}

	for (const auto & [surface, indices] : surfaceIndices) {
		auto meshItr = meshes.find(surface);
		if (meshItr == meshes.end()) {
			meshItr = meshes.emplace(surface, SurfaceArrayBuffer {});
			VertexArrayObject {meshItr->second.vertexArray}
					.addAttribsFor<Vertex>(verticesBuffer)
					.addIndices(meshItr->second.indicesBuffer, indices)
					.data(verticesBuffer, GL_ARRAY_BUFFER);
		}
		else {
			VertexArrayObject {meshItr->second.vertexArray}
					.addIndices(meshItr->second.indicesBuffer, indices)
					.data(verticesBuffer, GL_ARRAY_BUFFER);
		}
		meshItr->second.count = static_cast<GLsizei>(indices.size());
	}
	if (meshes.size() > surfaceIndices.size()) {
		std::erase_if(meshes, [&surfaceIndices](const auto & mesh) {
			return !surfaceIndices.contains(mesh.first);
		});
	}
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
	grass->bind();
	for (const auto & [surface, sab] : meshes) {
		shader.landmass.use(surface ? surface->colorBias : OPEN_SURFACE);
		glBindVertexArray(sab.vertexArray);
		glDrawElements(GL_TRIANGLES, sab.count, GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);
}

void
Terrain::shadows(const ShadowMapper & shadowMapper) const
{
	shadowMapper.landmess.use();
	for (const auto & [surface, sab] : meshes) {
		glBindVertexArray(sab.vertexArray);
		glDrawElements(GL_TRIANGLES, sab.count, GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);
}
