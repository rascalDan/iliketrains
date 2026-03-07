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

	static void configureBillBoardTextures(glTextures<3> &, GLsizei width, GLsizei height);
	void setView(Angle angle, const glm::mat4 &);
	[[nodiscard]] Angle getAngle() const;
	void renderBillBoard(const glTextures<3> &, const MeshBase &, Texture::AnyPtr texture) const;

private:
	glFrameBuffer fbo;
	Program program;
	Program::RequiredUniformLocation viewProjectionLoc {program, "viewProjection"};
	Program::RequiredUniformLocation viewLoc {program, "view"};

	Angle angle;
	glm::mat4 view;
};
