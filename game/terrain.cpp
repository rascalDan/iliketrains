#include "terrain.h"
#include "gfx/frustum.h"
#include <algorithm>
#include <gfx/gl/sceneShader.h>
#include <gfx/gl/shadowMapper.h>
#include <gfx/image.h>
#include <gfx/models/mesh.h>
#include <gfx/models/vertex.h>
#include <glMappedBufferSpan.h>
#include <glm/glm.hpp>
#include <location.h>
#include <maths.h>
#include <utility>
#include <vector>

static constexpr RGB OPEN_SURFACE {-1};
static constexpr GlobalDistance TILE_SIZE = 1024 * 1024; // ~1km, power of 2, fast divide

void
Terrain::initialise()
{
	glDebugScope _ {0};
	vertexArray.configure().addAttribs<Terrain::Vertex, &Terrain::Vertex::pos, &Terrain::Vertex::normal>(
			0, verticesBuffer);
	generateMeshes();
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
	std::ranges::transform(all_vertices(),
			glMappedBufferSpan<Vertex> {verticesBuffer, n_vertices(), GL_WRITE_ONLY, true}.begin(),
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
		auto & mesh = meshes[surfaceKey];
		mesh.indicesBuffer.data(indices, GL_DYNAMIC_DRAW);
		mesh.count = static_cast<GLsizei>(indices.size());
		mesh.aabb = AxisAlignedBoundingBox<GlobalDistance>::fromPoints(
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
	glDebugScope _ {0};
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
	glDebugScope _ {0};
	glBindVertexArray(vertexArray);
	grass->bind(0);

	const auto chunkBySurface = std::views::chunk_by([](const auto & itr1, const auto & itr2) {
		return itr1.first.surface == itr2.first.surface;
	});
	for (const auto & surfaceRange : meshes | chunkBySurface) {
		const auto surface = surfaceRange.front().first.surface;
		shader.landmass.use(surface ? surface->colorBias : OPEN_SURFACE);
		for (const auto & sab : surfaceRange) {
			if (frustum.contains(sab.second.aabb)) {
				glVertexArrayElementBuffer(vertexArray, sab.second.indicesBuffer);
				glDrawElements(GL_TRIANGLES, sab.second.count, GL_UNSIGNED_INT, nullptr);
			}
		}
	}
	glBindVertexArray(0);
}

void
Terrain::shadows(const ShadowMapper & shadowMapper, const Frustum & frustum) const
{
	glDebugScope _ {0};
	glBindVertexArray(vertexArray);
	shadowMapper.landmess.use();
	for (const auto & [surface, sab] : meshes) {
		if (frustum.contains(sab.aabb)) {
			glVertexArrayElementBuffer(vertexArray, sab.indicesBuffer);
			glDrawElements(GL_TRIANGLES, sab.count, GL_UNSIGNED_INT, nullptr);
		}
	}
	glBindVertexArray(0);
}
