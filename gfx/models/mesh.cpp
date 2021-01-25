#include "mesh.h"
#include "obj_loader.h"
#include "vertex.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

Mesh::Mesh(const std::string & fileName) : Mesh(OBJModel(fileName).ToIndexedModel()) { }

Mesh::Mesh(const IndexedModel & model) :
	m_vertexArrayObject {}, m_vertexArrayBuffers {}, m_numIndices {model.indices.size()}
{
	glGenVertexArrays(1, &m_vertexArrayObject);
	glBindVertexArray(m_vertexArrayObject);

	glGenBuffers(NUM_BUFFERS, m_vertexArrayBuffers.data());

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffers[POSITION_VB]);
	glBufferData(
			GL_ARRAY_BUFFER, sizeof(model.positions[0]) * model.positions.size(), &model.positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffers[TEXCOORD_VB]);
	glBufferData(
			GL_ARRAY_BUFFER, sizeof(model.texCoords[0]) * model.texCoords.size(), &model.texCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(model.normals[0]) * model.normals.size(), &model.normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vertexArrayBuffers[INDEX_VB]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(model.indices[0]) * model.indices.size(), &model.indices[0],
			GL_STATIC_DRAW);

	glBindVertexArray(0);
}

Mesh::Mesh(Vertex * vertices, unsigned int numVertices, unsigned int * indices, unsigned int numIndices) :
	Mesh {[vertices, numVertices, indices, numIndices]() {
		IndexedModel model;

		for (unsigned int i = 0; i < numVertices; i++) {
			model.positions.push_back(vertices[i].pos);
			model.texCoords.push_back(vertices[i].texCoord);
			model.normals.push_back(vertices[i].normal);
		}

		for (unsigned int i = 0; i < numIndices; i++) {
			model.indices.push_back(indices[i]);
		}

		return model;
	}()}
{
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

	glDrawElementsBaseVertex(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr, 0);

	glBindVertexArray(0);
}
