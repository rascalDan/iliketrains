#ifndef SHADER_INCLUDED_H
#define SHADER_INCLUDED_H

#include <GL/glew.h>
#include <array>
#include <glRef.hpp>
#include <glm/glm.hpp>
#include <string_view>

class Shader {
public:
	enum class Program { Basic = 0, Water = 1, LandMass = 2 };

	Shader();

	void setView(glm::mat4 view) const;
	void setModel(glm::mat4 model, Program = Program::Basic) const;
	void setUniform(const GLchar *, glm::vec3 dir) const;

private:
	class Source {
	public:
		using ShaderRef = glRef<GLuint, __glewCreateShader, __glewDeleteShader>;

		Source(const std::basic_string_view<unsigned char> text, GLuint type);
		Source(const GLchar * text, GLint len, GLuint type);

		ShaderRef id;
	};

	static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage);

	class ProgramHandle {
	public:
		explicit ProgramHandle(std::initializer_list<GLuint>);
		using ProgramRef = glRef<GLuint, __glewCreateProgram, __glewDeleteProgram>;

		ProgramRef m_program;
		GLint viewProjection_uniform, model_uniform;
	};

	std::array<ProgramHandle, 3> programs;
};

#endif
