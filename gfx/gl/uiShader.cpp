#include "uiShader.h"
#include <gfx/gl/shaders/fs-uiShader.h>
#include <gfx/gl/shaders/vs-uiShader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

UIShader::UIShader(size_t width, size_t height) : program {uiShader_vs.compile(), uiShader_fs.compile()}
{
	if (auto loc = glGetUniformLocation(program.m_program, "uiProjection"); loc >= 0) {
		glUseProgram(program.m_program);
		const auto uiProjection = glm::ortho<float>(0, static_cast<float>(width), 0, static_cast<float>(height));
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(uiProjection));
	}
}
