#ifndef MESH_INCLUDED_H
#define MESH_INCLUDED_H

#include <GL/glew.h>
#include <array>
#include <cstddef>
#include <special_members.hpp>
#include <string>

class IndexedModel;
class Vertex;

enum MeshBufferPositions { POSITION_VB, TEXCOORD_VB, NORMAL_VB, INDEX_VB };

class Mesh {
public:
	explicit Mesh(const std::string & fileName);
	explicit Mesh(const IndexedModel & model);
	Mesh(Vertex * vertices, unsigned int numVertices, unsigned int * indices, unsigned int numIndices);
	virtual ~Mesh();

	NO_COPY(Mesh);
	NO_MOVE(Mesh);

	void Draw();

private:
	static constexpr unsigned int NUM_BUFFERS {4};

	GLuint m_vertexArrayObject;
	std::array<GLuint, NUM_BUFFERS> m_vertexArrayBuffers;
	size_t m_numIndices;
};

#endif
