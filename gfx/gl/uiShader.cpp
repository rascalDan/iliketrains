#include "uiShader.h"
#include "gl_traits.h"
#include <gfx/gl/program.h>
#include <gfx/gl/shader.h>
#include <gfx/gl/shaders/fs-uiShader.h>
#include <gfx/gl/shaders/fs-uiShaderFont.h>
#include <gfx/gl/shaders/vs-uiShader.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

UIShader::IconProgram::IconProgram(const glm::mat4 & vp) : UIProgram {vp, uiShader_vs, uiShader_fs} { }

UIShader::TextProgram::TextProgram(const glm::mat4 & vp) : UIProgram {vp, uiShader_vs, uiShaderFont_fs} { }

UIShader::UIShader(size_t width, size_t height) :
	UIShader {glm::ortho<float>(0, static_cast<float>(width), 0, static_cast<float>(height))}
{
}

UIShader::UIShader(const glm::mat4 & viewProjection) : icon {viewProjection}, text {viewProjection} { }

void
UIShader::TextProgram::use(const RGB & colour) const
{
	Program::use();
	glUniform(colorLoc, colour);
}
