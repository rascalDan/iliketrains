#pragma once

#include "config/types.h"
#include <glad/gl.h>

class Vertex {
public:
#ifndef __cpp_aggregate_paren_init
	constexpr Vertex(Position3D pos, TextureRelCoord texCoord, Normal3D normal, RGBA colour = {}, GLuint material = 0) :
		pos {std::move(pos)}, texCoord {std::move(texCoord)}, normal {std::move(normal)}, colour {std::move(colour)},
		material {material}
	{
	}
#endif

	bool operator==(const Vertex &) const = default;

	Position3D pos {};
	TextureRelCoord texCoord {};
	Normal3D normal {};
	RGBA colour {};
	GLuint material {};
};
