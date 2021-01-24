#ifndef SHADER_INCLUDED_H
#define SHADER_INCLUDED_H

#include <GL/glew.h>
#include <glRef.hpp>
#include <glm/glm.hpp>
#include <string_view>

class Shader {
public:
	Shader();

	void Bind() const;
	void setView(glm::mat4 view) const;
	void setModel(glm::mat4 model) const;

private:
	class Source {
	public:
		using ShaderRef = glRef<GLuint, __glewCreateShader, __glewDeleteShader>;

		Source(const std::basic_string_view<unsigned char> text, GLuint type);
		Source(const GLchar * text, GLint len, GLuint type);

		ShaderRef id;
	};

	static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage);

	using ProgramRef = glRef<GLuint, __glewCreateProgram, __glewDeleteProgram>;

	ProgramRef m_program;
	GLint viewProjection_uniform, model_uniform, lightDir_uniform;
};

#endif
