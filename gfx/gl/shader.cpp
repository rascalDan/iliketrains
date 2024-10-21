#include "shader.h"
#include "msgException.h"
#include <algorithm>
#include <array>
#include <format>
#include <stdexcept>
#include <string>

namespace {
	auto
	getInt(GLenum e)
	{
		GLint i {};
		glGetIntegerv(e, &i);
		return std::to_string(i);
	}

	using LookUpFunction = std::string (*)(GLenum);
	constexpr std::array<std::tuple<std::string_view, GLenum, LookUpFunction>, 1> LOOKUPS {{
			{"GL_MAX_GEOMETRY_OUTPUT_VERTICES", GL_MAX_GEOMETRY_OUTPUT_VERTICES, getInt},
	}};

	struct ShaderCompileError : public MsgException<std::invalid_argument> {
		explicit ShaderCompileError(GLuint shader, Shader::Source src) :
			MsgException<std::invalid_argument> {"Error compiling shader"}, shader {shader}, source {src},
			msg {getShaderText(GL_INFO_LOG_LENGTH, glGetShaderInfoLog)}
		{
		}

		[[nodiscard]] std::string
		getMsg() const noexcept override
		{
			return std::format("Error compiling shader: '{}'\nSource:\n{}",
					getShaderText(GL_INFO_LOG_LENGTH, glGetShaderInfoLog), source);
		}

	private:
		std::string
		getShaderText(GLenum param, auto getTextFunc) const
		{
			std::string text;
			text.resize_and_overwrite(static_cast<size_t>(Shader::getShaderParam(shader, param)),
					[this, getTextFunc](auto buf, auto len) {
						GLsizei outLen {};
						getTextFunc(shader, static_cast<GLsizei>(len), &outLen, buf);
						return outLen;
					});
			return text;
		}

		const GLuint shader;
		const Shader::Source source;
		const std::string msg;
	};
}

Shader::ShaderRef
Shader::compile() const
{
	ShaderRef shader {type};
	auto source = [&shader](auto text, GLint len) {
		glShaderSource(shader, 1, &text, &len);
	};
	if (lookups) {
		std::basic_string<GLchar> textMod {text};
		for (const auto & match : ctre::search_all<R"(\bGL_[A-Z_]+\b)">(textMod)) {
			if (const auto * const lookup = std::find_if(LOOKUPS.begin(), LOOKUPS.end(),
						[&match](const auto & lookup) {
							return std::get<std::string_view>(lookup) == match;
						});
					lookup != LOOKUPS.end()) {
				const auto & [name, pname, getFunction] = *lookup;
				textMod.replace(match.begin(), match.end(), getFunction(pname));
			}
		}
		source(textMod.c_str(), static_cast<GLint>(textMod.length()));
	}
	else {
		source(text.data(), static_cast<GLint>(text.length()));
	}
	glCompileShader(shader);

	checkShaderError(shader);
	return shader;
}

void
Shader::checkShaderError(GLuint shader) const
{
	if (getShaderParam(shader, GL_COMPILE_STATUS) == GL_FALSE) {
		throw ShaderCompileError {shader, text};
	}
}

GLint
Shader::getShaderParam(GLuint shader, GLenum pname)
{
	GLint pvalue {};
	glGetShaderiv(shader, pname, &pvalue);
	return pvalue;
}
