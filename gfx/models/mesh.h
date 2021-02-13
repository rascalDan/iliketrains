#ifndef MESH_INCLUDED_H
#define MESH_INCLUDED_H

#include <GL/glew.h>
#include <array>
#include <cstddef>
#include <filesystem>
#include <gfx/models/vertex.hpp>
#include <span>
#include <special_members.hpp>
#include <utility>
#include <vector>

class ObjParser;

enum MeshBufferPositions { POSITION_VB, TEXCOORD_VB, NORMAL_VB, INDEX_VB };

class Mesh {
public:
	using Data = std::pair<std::vector<Vertex>, std::vector<unsigned int>>;
	explicit Mesh(const std::filesystem::path & fileName);
	explicit Mesh(const ObjParser & obj);
	Mesh(std::span<Vertex> vertices, std::span<unsigned int> indices, GLenum = GL_TRIANGLES);
	virtual ~Mesh();

	NO_COPY(Mesh);
	NO_MOVE(Mesh);

	void Draw() const;

private:
	explicit Mesh(Data && vandi, GLenum = GL_TRIANGLES);

	static Data packObjParser(const ObjParser &);

	static constexpr unsigned int NUM_BUFFERS {4};

	GLuint m_vertexArrayObject;
	std::array<GLuint, NUM_BUFFERS> m_vertexArrayBuffers;
	size_t m_numIndices;
	GLenum mode;
};

#endif
