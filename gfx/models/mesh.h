#ifndef MESH_INCLUDED_H
#define MESH_INCLUDED_H

#include <GL/glew.h>
#include <array>
#include <memory>
#include <span>
#include <special_members.hpp>

class Vertex;

enum MeshBufferPositions { POSITION_VB, TEXCOORD_VB, NORMAL_VB, INDEX_VB };

class Mesh {
public:
	Mesh(const std::span<const Vertex> vertices, const std::span<const unsigned int> indices, GLenum = GL_TRIANGLES);
	virtual ~Mesh();

	NO_COPY(Mesh);
	NO_MOVE(Mesh);

	void Draw() const;

private:
	static constexpr unsigned int NUM_BUFFERS {4};

	GLuint m_vertexArrayObject;
	std::array<GLuint, NUM_BUFFERS> m_vertexArrayBuffers;
	GLsizei m_numIndices;
	GLenum mode;
};
using MeshPtr = std::shared_ptr<const Mesh>;

#endif
