#include "uiShader.h"
#include <gfx/gl/glSource.h>
#include <gfx/gl/programHandle.h>
#include <gfx/gl/shaders/fs-uiShader.h>
#include <gfx/gl/shaders/fs-uiShaderFont.h>
#include <gfx/gl/shaders/vs-uiShader.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <initializer_list>

UIShader::UIShader(size_t width, size_t height) :
	progDefault {uiShader_vs.compile(), uiShader_fs.compile()}, progText {uiShader_vs.compile(),
																		uiShaderFont_fs.compile()}
{
	for (const auto prog : {&progDefault, &progText}) {
		if (auto loc = glGetUniformLocation(prog->m_program, "uiProjection"); loc >= 0) {
			glUseProgram(prog->m_program);
			const auto uiProjection = glm::ortho<float>(0, static_cast<float>(width), 0, static_cast<float>(height));
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(uiProjection));
		}
	}
}

void
UIShader::useDefault() const
{
	glUseProgram(progDefault.m_program);
}

void
UIShader::useText(glm::vec3 colour) const
{
	glUseProgram(progText.m_program);
	if (auto loc = glGetUniformLocation(progText.m_program, "colour"); loc >= 0) {
		glUniform3fv(loc, 1, glm::value_ptr(colour));
	}
}
