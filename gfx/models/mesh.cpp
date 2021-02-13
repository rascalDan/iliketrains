#include "mesh.h"
#include "obj.h"
#include "vertex.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <iterator>
#include <resource.h>
#include <vector>

Mesh::Mesh(const std::filesystem::path & fileName) : Mesh(ObjParser {Resource::mapPath(fileName)}) { }

Mesh::Mesh(const ObjParser & obj) : Mesh(packObjParser(obj), GL_TRIANGLES) { }

Mesh::Mesh(std::pair<std::vector<Vertex>, std::vector<unsigned int>> && vandi, GLenum m) :
	Mesh(vandi.first, vandi.second, m)
{
}

Mesh::Mesh(std::span<Vertex> vertices, std::span<unsigned int> indices, GLenum m) :
	m_vertexArrayObject {}, m_vertexArrayBuffers {}, m_numIndices {indices.size()}, mode {m}
{
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

Mesh::Data
Mesh::packObjParser(const ObjParser & obj)
{
	std::vector<Vertex> vertices;
	std::vector<ObjParser::FaceElement> vertexOrder;
	std::vector<unsigned int> indices;
	std::for_each(obj.faces.begin(), obj.faces.end(), [&](const ObjParser::Face & face) {
		for (auto idx = 2U; idx < face.size(); idx += 1) {
			auto f = [&](auto idx) {
				const auto & fe {face[idx]};
				if (const auto existing = std::find(vertexOrder.begin(), vertexOrder.end(), fe);
						existing != vertexOrder.end()) {
					indices.push_back(std::distance(vertexOrder.begin(), existing));
				}
				else {
					indices.push_back(vertices.size());
					vertices.emplace_back(obj.vertices[fe.x - 1], obj.texCoords[fe.y - 1], -obj.normals[fe.z - 1]);
					vertexOrder.emplace_back(fe);
				}
			};
			f(0);
			f(idx);
			f(idx - 1);
		}
	});
	return std::make_pair(vertices, indices);
}

Mesh::~Mesh()
{
	glDeleteBuffers(NUM_BUFFERS, m_vertexArrayBuffers.data());
	glDeleteVertexArrays(1, &m_vertexArrayObject);
}

void
Mesh::Draw() const
{
	glBindVertexArray(m_vertexArrayObject);

	glDrawElementsBaseVertex(mode, m_numIndices, GL_UNSIGNED_INT, nullptr, 0);

	glBindVertexArray(0);
}
