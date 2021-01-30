#include "terrain.h"
#include "gfx/models/texture.h"
#include <array>
#include <cache.h>
#include <cmath>
#include <cstddef>
#include <gfx/gl/shader.h>
#include <gfx/gl/transform.h>
#include <gfx/image.h>
#include <gfx/models/vertex.hpp>
#include <glm/glm.hpp>
#include <random>
#include <stb_image.h>

Terrain::Terrain() :
	m_vertexArrayObject {}, m_vertexArrayBuffers {}, texture {Texture::cachedTexture.get("terrain.png")}
{
	constexpr auto size {241}; // Vertices
	constexpr auto offset {(size - 1) / 2};
	constexpr auto verticesCount = size * size;
	constexpr auto resolution = 10; // Grid size

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	vertices.reserve(verticesCount + 4);
	vertices.resize(verticesCount, {{}, {}, {}});

	// Initial coordinates
	for (auto z = 0; z < size; z += 1) {
		for (auto x = 0; x < size; x += 1) {
			auto & vertex = vertices[x + (z * size)];
			vertex.pos = {resolution * (x - offset), -1.5, resolution * (z - offset)};
			vertex.normal = {0, 1, 0};
			vertex.texCoord = {(x % 2) / 2.01, (z % 2) / 2.01};
		}
	}
	// Add hills
	std::mt19937 gen(std::random_device {}());
	std::uniform_int_distribution<> rpos(2, size - 2);
	std::uniform_int_distribution<> rsize(10, 20);
	std::uniform_int_distribution<> rheight(1, 3);
	for (int h = 0; h < 500;) {
		const glm::ivec2 hpos {rpos(gen), rpos(gen)};
		const glm::ivec2 hsize {rsize(gen), rsize(gen)};
		if (const auto lim1 = hpos - hsize; lim1.x > 0 && lim1.y > 0) {
			if (const auto lim2 = hpos + hsize; lim2.x < size && lim2.y < size) {
				auto height {rheight(gen)};
				const glm::ivec2 hsizesqrd {hsize.x * hsize.x, hsize.y * hsize.y};
				for (auto z = lim1.y; z < lim2.y; z += 1) {
					for (auto x = lim1.x; x < lim2.x; x += 1) {
						const auto dist {hpos - glm::ivec2 {x, z}};
						const glm::ivec2 distsqrd {dist.x * dist.x, dist.y * dist.y};
						if ((pow(x - hpos.x, 2) / pow(hsize.x, 2)) + (pow(z - hpos.y, 2) / pow(hsize.y, 2)) <= 1.0) {
							auto & vertex = vertices[x + (z * size)];
							vertex.pos.y += height;
						}
					}
				}
				h += 1;
			}
		}
	}
	finish(size, size, resolution);
}

Terrain::Terrain(const std::string & fileName) :
	m_vertexArrayObject {}, m_vertexArrayBuffers {}, texture {Texture::cachedTexture.get("terrain.png")}
{
	constexpr auto resolution {100};

	const Image map {fileName.c_str(), STBI_grey};

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	vertices.reserve((map.width * map.height) + 4);

	for (auto z = 0; z < map.height; z += 1) {
		for (auto x = 0; x < map.width; x += 1) {
			vertices.emplace_back(
					glm::vec3 {resolution * (x - (map.width / 2)), ((float)map.data[x + (z * map.width)] * 0.1F) - 1.5F,
							resolution * (z - (map.height / 2))},
					glm::vec2 {(x % 2) / 2.01, (z % 2) / 2.01}, glm::vec3 {0, 1, 0});
		}
	}

	finish(map.width, map.height, resolution);
}

void
Terrain::finish(unsigned int width, unsigned int height, unsigned int resolution)
{
	const auto tilesCount = (width - 1) * (height - 1);
	const auto trianglesCount = tilesCount * 2;
	const auto indicesCount = trianglesCount * 3;
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
	for (auto z = 1U; z < height - 1; z += 1) {
		for (auto x = 1U; x < width - 1; x += 1) {
			const auto a = v(width, x - 1, z).pos;
			const auto b = v(width, x, z - 1).pos;
			const auto c = v(width, x + 1, z).pos;
			const auto d = v(width, x, z + 1).pos;
			v(width, x, z).normal = -glm::normalize(glm::cross(c - a, d - b));
		}
	}
	const auto verticesCount = vertices.size();
	// Add water
	const auto extentx {(int)((width - 1) * resolution / 2)};
	const auto extentz {(int)((height - 1) * resolution / 2)};
	vertices.emplace_back(glm::vec3 {-extentx, 0, -extentz}, glm::vec2 {0.5, 0.0}, glm::vec3 {0, 1, 0});
	vertices.emplace_back(glm::vec3 {-extentx, 0, extentz}, glm::vec2 {0.5, 0.5}, glm::vec3 {0, 1, 0});
	vertices.emplace_back(glm::vec3 {extentx, 0, extentz}, glm::vec2 {1, 0.5}, glm::vec3 {0, 1, 0});
	vertices.emplace_back(glm::vec3 {extentx, 0, -extentz}, glm::vec2 {1, 0.0}, glm::vec3 {0, 1, 0});
	indices.push_back(verticesCount);
	indices.push_back(verticesCount + 1);
	indices.push_back(verticesCount + 2);
	indices.push_back(verticesCount);
	indices.push_back(verticesCount + 2);
	indices.push_back(verticesCount + 3);

	glGenVertexArrays(1, &m_vertexArrayObject);
	glBindVertexArray(m_vertexArrayObject);

	glGenBuffers(2, m_vertexArrayBuffers.data());

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexArrayBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

Vertex &
Terrain::v(unsigned int width, unsigned int x, unsigned int z)
{
	return vertices[x + (z * width)];
}

Terrain::~Terrain()
{
	glDeleteBuffers(NUM_BUFFERS, m_vertexArrayBuffers.data());
	glDeleteVertexArrays(1, &m_vertexArrayObject);
}

static const Transform identity {};
static const auto identityModel {identity.GetModel()};

void
Terrain::render(const Shader & shader) const
{
	shader.setModel(identityModel);
	texture->Bind();
	glBindVertexArray(m_vertexArrayObject);

	glDrawElementsBaseVertex(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, 0);

	glBindVertexArray(0);
}
