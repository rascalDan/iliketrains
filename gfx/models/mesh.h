#ifndef MESH_INCLUDED_H
#define MESH_INCLUDED_H

#include <GL/glew.h>
#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <special_members.hpp>

class Vertex;

enum MeshBufferPositions { POSITION_VB, TEXCOORD_VB, NORMAL_VB, INDEX_VB };

class Mesh {
public:
	Mesh(std::span<Vertex> vertices, std::span<unsigned int> indices, GLenum = GL_TRIANGLES);
	virtual ~Mesh();

	NO_COPY(Mesh);
	NO_MOVE(Mesh);

	void Draw() const;

private:
	static constexpr unsigned int NUM_BUFFERS {4};

	GLuint m_vertexArrayObject;
	std::array<GLuint, NUM_BUFFERS> m_vertexArrayBuffers;
	size_t m_numIndices;
	GLenum mode;
};
using MeshPtr = std::shared_ptr<const Mesh>;

#endif
