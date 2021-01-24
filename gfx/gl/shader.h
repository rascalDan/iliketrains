#ifndef SHADER_INCLUDED_H
#define SHADER_INCLUDED_H

#include <GL/glew.h>
#include <glRef.hpp>
#include <string_view>

class Camera;
class Transform;

class Shader {
public:
	Shader();

	void Bind() const;
	void Update(const Transform & transform, const Camera & camera) const;

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
	GLint mvp_uniform, normal_uniform, lightDir_uniform;
};

#endif
