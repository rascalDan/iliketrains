#include "terrain.h"
#include "gfx/frustum.h"
#include <algorithm>
#include <gfx/gl/sceneShader.h>
#include <gfx/gl/shadowMapper.h>
#include <gfx/image.h>
#include <gfx/models/mesh.h>
#include <gfx/models/vertex.h>
#include <glMappedBufferWriter.h>
#include <glm/glm.hpp>
#include <location.h>
#include <maths.h>
#include <utility>
#include <vector>

static constexpr RGB OPEN_SURFACE {-1};
static constexpr GlobalDistance TILE_SIZE = 1024 * 1024; // ~1km, power of 2, fast divide

template<>
VertexArrayObject &
VertexArrayObject::addAttribsFor<Terrain::Vertex>(const GLuint arrayBuffer, const GLuint divisor)
{
	return addAttribs<Terrain::Vertex, &Terrain::Vertex::pos, &Terrain::Vertex::normal>(arrayBuffer, divisor);
}

bool
Terrain::SurfaceKey::operator<(const SurfaceKey & other) const
{
	return std::tie(surface, basePosition.x, basePosition.y)
			< std::tie(other.surface, other.basePosition.x, other.basePosition.y);
}

inline void
Terrain::copyVerticesToBuffer() const
{
	std::ranges::transform(all_vertices(), glMappedBufferWriter<Vertex> {GL_ARRAY_BUFFER, verticesBuffer, n_vertices()},
			[this](const auto & vertex) {
				return Vertex {point(vertex), normal(vertex)};
			});
}

inline GlobalPosition2D
Terrain::getTile(const FaceHandle & face) const
{
	return point(*cfv_begin(face)).xy() / TILE_SIZE;
};

Terrain::SurfaceIndices
Terrain::mapSurfaceFacesToIndices() const
{
	SurfaceIndices surfaceIndices;
	const auto indexBySurfaceAndTile = std::views::transform([this](const auto & faceItr) {
		return std::pair<SurfaceKey, FaceHandle> {{getSurface(*faceItr), getTile(*faceItr)}, *faceItr};
	});
	const auto chunkBySurfaceAndTile = std::views::chunk_by([](const auto & face1, const auto & face2) {
		return face1.first.surface == face2.first.surface && face1.first.basePosition == face2.first.basePosition;
	});
	for (const auto & faceRange : faces() | indexBySurfaceAndTile | chunkBySurfaceAndTile) {
		const SurfaceKey & surfaceKey = faceRange.front().first;
		auto indexItr = surfaceIndices.find(surfaceKey);
		if (indexItr == surfaceIndices.end()) {
			indexItr = surfaceIndices.emplace(surfaceKey, std::vector<GLuint> {}).first;
			if (auto existing = meshes.find(surfaceKey); existing != meshes.end()) {
				indexItr->second.reserve(static_cast<size_t>(existing->second.count));
			}
		}
		for (auto push = std::back_inserter(indexItr->second); const auto & [_, face] : faceRange) {
			std::ranges::transform(fv_range(face), push, &OpenMesh::VertexHandle::idx);
		}
	}
	return surfaceIndices;
}

void
Terrain::copyIndicesToBuffers(const SurfaceIndices & surfaceIndices)
{
	for (const auto & [surfaceKey, indices] : surfaceIndices) {
		auto meshItr = meshes.find(surfaceKey);
		if (meshItr == meshes.end()) {
			meshItr = meshes.emplace(surfaceKey, SurfaceArrayBuffer {}).first;
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
		meshItr->second.aabb = AxisAlignedBoundingBox<GlobalDistance>::fromPoints(
				indices | std::views::transform([this](const auto vertex) {
					return this->point(VertexHandle {static_cast<int>(vertex)});
				}));
	}
}

void
Terrain::pruneOrphanMeshes(const SurfaceIndices & surfaceIndices)
{
	if (meshes.size() > surfaceIndices.size()) {
		std::erase_if(meshes, [&surfaceIndices](const auto & mesh) {
			return !surfaceIndices.contains(mesh.first);
		});
	}
}

void
Terrain::generateMeshes()
{
	copyVerticesToBuffer();
	const auto surfaceIndices = mapSurfaceFacesToIndices();
	copyIndicesToBuffers(surfaceIndices);
	pruneOrphanMeshes(surfaceIndices);
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
Terrain::render(const SceneShader & shader, const Frustum & frustum) const
{
	grass->bind();

	const auto chunkBySurface = std::views::chunk_by([](const auto & itr1, const auto & itr2) {
		return itr1.first.surface == itr2.first.surface;
	});
	for (const auto & surfaceRange : meshes | chunkBySurface) {
		const auto surface = surfaceRange.front().first.surface;
		shader.landmass.use(surface ? surface->colorBias : OPEN_SURFACE);
		for (const auto & sab : surfaceRange) {
			if (frustum.contains(sab.second.aabb)) {
				glBindVertexArray(sab.second.vertexArray);
				glDrawElements(GL_TRIANGLES, sab.second.count, GL_UNSIGNED_INT, nullptr);
			}
		}
	}
	glBindVertexArray(0);
}

void
Terrain::shadows(const ShadowMapper & shadowMapper, const Frustum & frustum) const
{
	shadowMapper.landmess.use();
	for (const auto & [surface, sab] : meshes) {
		if (frustum.shadedBy(sab.aabb)) {
			glBindVertexArray(sab.vertexArray);
			glDrawElements(GL_TRIANGLES, sab.count, GL_UNSIGNED_INT, nullptr);
		}
	}
	glBindVertexArray(0);
}
