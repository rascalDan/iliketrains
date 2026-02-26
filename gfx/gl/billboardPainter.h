#pragma once

#include "gfx/gl/program.h"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include "glArrays.h"

class LightDirection;

class BillboardPainter {
public:
	template<typename T> static constexpr T VIEW_ANGLES = 8;

	BillboardPainter();

	[[nodiscard]]
	static glTextures<3> createBillBoardTextures(GLsizei width, GLsizei height);
	void setView(const glm::mat4 &);
	void renderBillBoard(const glTextures<3> &, const MeshBase &, Texture::AnyPtr texture) const;

private:
	glFrameBuffer fbo;
	Program program;
	Program::RequiredUniformLocation viewProjectionLoc {program, "viewProjection"};
	Program::RequiredUniformLocation viewLoc {program, "view"};

	glm::mat4 view;
};
