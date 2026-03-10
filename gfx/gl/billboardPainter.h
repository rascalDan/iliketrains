#pragma once

#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include "glFramebuffer.h"
#include "program.h"

class LightDirection;

class BillboardPainter {
public:
	template<typename T> static constexpr T VIEW_ANGLES = 8;

	BillboardPainter();

	static void configureBillBoardTextures(glTextures<GL_TEXTURE_2D_ARRAY, 3> &, ImageDimensions);
	void setView(Angle angle, const glm::mat4 &);
	[[nodiscard]] Angle getAngle() const;
	void renderBillBoard(const glTextures<GL_TEXTURE_2D_ARRAY, 3> &, const MeshBase &, Texture::AnyPtr texture) const;

private:
	mutable glFramebuffer fbo;
	Program program;
	Program::RequiredUniformLocation viewProjectionLoc {program, "viewProjection"};
	Program::RequiredUniformLocation viewLoc {program, "view"};

	Angle angle;
	glm::mat4 view;
};
