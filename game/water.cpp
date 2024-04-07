#include "water.h"
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
#include <set>
#include <utility>
#include <vector>

namespace glm {
	bool
	operator<(const GlobalPosition2D a, const GlobalPosition2D b)
	{
		return std::tie(a.x, a.y) < std::tie(b.x, b.y);
	}
}

Water::Water(std::shared_ptr<GeoData> tm) : geoData {std::move(tm)}, water {std::make_shared<Texture>("water.png")}
{
	generateMeshes();
}

static constexpr GlobalDistance MIN_HEIGHT = 1'000;
static constexpr GlobalDistance TILE_SIZE = 8'192;
static constexpr GlobalDistance BORDER = TILE_SIZE / 2;

void
Water::generateMeshes()
{
	// Map out where a water square needs to exist to cover all terrain faces with a low vertex
	std::set<GlobalPosition2D> waterPositions;
	std::for_each(geoData->vertices_sbegin(), geoData->vertices_end(), [this, &waterPositions](const auto vh) {
		if (geoData->point(vh).z < MIN_HEIGHT) {
			std::for_each(geoData->vf_begin(vh), geoData->vf_end(vh),
					[done = std::set<OpenMesh::FaceHandle>(), this, &waterPositions](const auto fh) mutable {
						if (done.insert(fh).second) {
							const auto getrange = [this, fh](glm::length_t axis) {
								const auto mme = std::minmax_element(geoData->fv_begin(fh), geoData->fv_end(fh),
										[this, axis](const auto vh1, const auto vh2) {
											return geoData->point(vh1)[axis] < geoData->point(vh2)[axis];
										});

								return std::make_pair((geoData->point(*mme.first)[axis] - BORDER) / TILE_SIZE,
										(geoData->point(*mme.second)[axis] + BORDER) / TILE_SIZE);
							};
							const auto xrange = getrange(0);
							const auto yrange = getrange(1);
							for (auto x = xrange.first; x < xrange.second; x++) {
								for (auto y = yrange.first; y < yrange.second; y++) {
									waterPositions.emplace(x, y);
								}
							}
						}
					});
		}
	});

	std::vector<unsigned int> indices;
	std::vector<Vertex> vertices;
	std::map<GlobalPosition2D, size_t> vertexIndex;
	std::for_each(waterPositions.begin(), waterPositions.end(),
			[&indices, &vertices, &vertexIndex, extents = geoData->getExtents(), this](const GlobalPosition2D p) {
				std::array<unsigned int, 4> currentIndices {};
				auto out = currentIndices.begin();
				for (auto x : {0, TILE_SIZE}) {
					for (auto y : {0, TILE_SIZE}) {
						const auto pos = (p * TILE_SIZE) + GlobalPosition2D {x, y};
						const auto v = vertexIndex.emplace(pos, vertices.size());
						if (v.second) {
							const auto cpos = glm::clamp(pos, std::get<0>(extents).xy(), std::get<1>(extents).xy());
							vertices.emplace_back(RelativePosition3D {geoData->positionAt(cpos)},
									TextureRelCoord(pos / TILE_SIZE), up);
						}
						*out++ = static_cast<unsigned int>(v.first->second);
					}
				}
				for (const auto i : {0U, 3U, 1U, 0U, 2U, 3U}) {
					indices.push_back(currentIndices[i]);
				}
			});
	meshes.create<Mesh>(vertices, indices);
}

void
Water::tick(TickDuration dur)
{
	waveCycle += dur.count();
}

void
Water::render(const SceneShader & shader) const
{
	shader.water.use(waveCycle);
	water->bind();
	meshes.apply(&Mesh::Draw);
}
