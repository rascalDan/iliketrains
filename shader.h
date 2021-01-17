#ifndef SHADER_INCLUDED_H
#define SHADER_INCLUDED_H

#include <GL/glew.h>
#include <array>
#include <string>

class Camera;
class Transform;

class Shader {
public:
	explicit Shader(const std::string & fileName);
	virtual ~Shader();
	void operator=(const Shader &) = delete;
	Shader(const Shader &) = delete;

	void Bind() const;
	void Update(const Transform & transform, const Camera & camera) const;

private:
	static constexpr unsigned int NUM_SHADERS = 2;
	static constexpr unsigned int NUM_UNIFORMS = 3;

	static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string & errorMessage);
	static GLuint CreateShader(const GLchar * text, GLint len, unsigned int type);

	GLuint m_program;
	std::array<GLuint, NUM_SHADERS> m_shaders;
	std::array<GLint, NUM_UNIFORMS> m_uniforms;
};

#endif
