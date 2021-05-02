#include "terrain.h"
#include "gfx/models/texture.h"
#include <cache.h>
#include <gfx/gl/shader.h>
#include <gfx/image.h>
#include <gfx/models/mesh.h>
#include <gfx/models/vertex.hpp>
#include <glm/glm.hpp>
#include <location.hpp>
#include <maths.h>
#include <random>
#include <stb_image.h>

Terrain::Terrain() : grass {Texture::cachedTexture.get("grass.png")}, water {Texture::cachedTexture.get("water.png")}
{
	constexpr auto size {241}; // Vertices
	constexpr auto offset {(size - 1) / 2};
	constexpr auto verticesCount = size * size;
	constexpr auto resolution = 10; // Grid size

	std::vector<Vertex> vertices;
	vertices.reserve(verticesCount + 4);
	vertices.resize(verticesCount, {{}, {}, {}});

	// Initial coordinates
	for (auto z = 0; z < size; z += 1) {
		for (auto x = 0; x < size; x += 1) {
			auto & vertex = vertices[x + (z * size)];
			vertex.pos = {resolution * (x - offset), -1.5, resolution * (z - offset)};
			vertex.normal = {0, 1, 0};
			vertex.texCoord = {x, z};
		}
	}
	// Add hills
	std::mt19937 gen(std::random_device {}());
	std::uniform_int_distribution<> rpos(2, size - 2);
	std::uniform_int_distribution<> rsize(10, 30);
	std::uniform_int_distribution<> rheight(1, 3);
	for (int h = 0; h < 500;) {
		const glm::ivec2 hpos {rpos(gen), rpos(gen)};
		const glm::ivec2 hsize {rsize(gen), rsize(gen)};
		if (const auto lim1 = hpos - hsize; lim1.x > 0 && lim1.y > 0) {
			if (const auto lim2 = hpos + hsize; lim2.x < size && lim2.y < size) {
				const auto height = (float)rheight(gen);
				const glm::ivec2 hsizesqrd {hsize.x * hsize.x, hsize.y * hsize.y};
				for (auto z = lim1.y; z < lim2.y; z += 1) {
					for (auto x = lim1.x; x < lim2.x; x += 1) {
						const auto dist {hpos - glm::ivec2 {x, z}};
						const glm::ivec2 distsqrd {dist.x * dist.x, dist.y * dist.y};
						const auto out {rdiv(sq(x - hpos.x), sq(hsize.x)) + rdiv(sq(z - hpos.y), sq(hsize.y))};
						if (out <= 1.0) {
							auto & vertex = vertices[x + (z * size)];
							const auto m {1.F / (7.F * out - 8.F) + 1.F};
							vertex.pos.y += height * m;
						}
					}
				}
				h += 1;
			}
		}
	}
	finish(size, size, vertices);
}

Terrain::Terrain(const std::string & fileName) :
	grass {Texture::cachedTexture.get("grass.png")}, water {Texture::cachedTexture.get("water.png")}
{
	constexpr auto resolution {100};

	const Image map {fileName.c_str(), STBI_grey};

	std::vector<Vertex> vertices;
	vertices.reserve((map.width * map.height) + 4);

	for (auto z = 0; z < map.height; z += 1) {
		for (auto x = 0; x < map.width; x += 1) {
			vertices.emplace_back(
					glm::vec3 {resolution * (x - (map.width / 2)), ((float)map.data[x + (z * map.width)] * 0.1F) - 1.5F,
							resolution * (z - (map.height / 2))},
					glm::vec2 {(x % 2) / 2.01, (z % 2) / 2.01}, glm::vec3 {0, 1, 0});
		}
	}

	finish(map.width, map.height, vertices);
}

void
Terrain::finish(unsigned int width, unsigned int height, std::vector<Vertex> & vertices)
{
	const auto tilesCount = (width - 1) * (height - 1);
	const auto trianglesCount = tilesCount * 2;
	const auto indicesCount = trianglesCount * 3;
	std::vector<unsigned int> indices;
	indices.reserve(indicesCount + 6);
	// Indices
	for (auto z = 0U; z < height - 1; z += 1) {
		for (auto x = 0U; x < width - 1; x += 1) {
			indices.push_back(x + (z * width));
			indices.push_back((x + 1) + ((z + 1) * width));
			indices.push_back((x + 1) + (z * width));
			indices.push_back(x + (z * width));
			indices.push_back(x + ((z + 1) * width));
			indices.push_back((x + 1) + ((z + 1) * width));
		}
	}
	// Normals
	auto v = [&vertices](unsigned int width, unsigned int x, unsigned int z) -> Vertex & {
		return vertices[x + (z * width)];
	};

	for (auto z = 1U; z < height - 1; z += 1) {
		for (auto x = 1U; x < width - 1; x += 1) {
			const auto a = v(width, x - 1, z).pos;
			const auto b = v(width, x, z - 1).pos;
			const auto c = v(width, x + 1, z).pos;
			const auto d = v(width, x, z + 1).pos;
			v(width, x, z).normal = -glm::normalize(glm::cross(c - a, d - b));
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
Terrain::render(const Shader & shader) const
{
	shader.setModel(Location {}, Shader::Program::LandMass);
	grass->Bind();
	meshes.apply(&Mesh::Draw);

	shader.setModel(Location {}, Shader::Program::Water);
	shader.setUniform("waves", {waveCycle, 0, 0});
	water->Bind();
	meshes.apply(&Mesh::Draw);
}
