#include "terrain.h"
#include "gfx/models/texture.h"
#include <array>
#include <cache.h>
#include <cstddef>
#include <gfx/gl/shader.h>
#include <gfx/gl/transform.h>
#include <gfx/models/vertex.hpp>
#include <glm/glm.hpp>

constexpr auto size {255}; // Vertices
constexpr auto verticesCount = size * size;
constexpr auto tilesCount = (size - 1) * (size - 1);
constexpr auto trianglesCount = tilesCount * 2;
constexpr auto indicesCount = trianglesCount * 3;
constexpr auto resolution = 10; // Grid size

Terrain::Terrain() :
	m_vertexArrayObject {}, m_vertexArrayBuffers {}, texture {Texture::cachedTexture.get("res/bricks.jpg")}
{
	vertices.resize(verticesCount, {{}, {}, {}});
	indices.reserve(indicesCount);

	// Initial coordinates
	for (auto z = 0; z < size; z += 1) {
		for (auto x = 0; x < size; x += 1) {
			auto & vertex = vertices[x + (z * size)];
			vertex.pos = {resolution * x, -1, resolution * z};
			vertex.normal = {0, 1, 0};
			vertex.texCoord = {x % 2, z % 2};
		}
	}
	// Indices
	for (auto z = 0; z < size - 1; z += 1) {
		for (auto x = 0; x < size - 1; x += 1) {
			indices.push_back(x + (z * size));
			indices.push_back((x + 1) + ((z + 1) * size));
			indices.push_back((x + 1) + (z * size));
			indices.push_back(x + (z * size));
			indices.push_back(x + ((z + 1) * size));
			indices.push_back((x + 1) + ((z + 1) * size));
		}
	}

	glGenVertexArrays(1, &m_vertexArrayObject);
	glBindVertexArray(m_vertexArrayObject);

	glGenBuffers(2, m_vertexArrayBuffers.data());

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * verticesCount, vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexArrayBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesCount, indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
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

	glDrawElementsBaseVertex(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, nullptr, 0);

	glBindVertexArray(0);
}
