#include "shader.h"
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
		for (const auto & match : ctre::range<R"(\bGL_[A-Z_]+\b)">(textMod)) {
			if (const auto lookup = std::find_if(LOOKUPS.begin(), LOOKUPS.end(),
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

	CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error compiling shader!");
	return shader;
}

void
Shader::CheckShaderError(GLuint shader, GLuint flag, bool isProgram, std::string_view errorMessage)
{
	GLint success = 0;

	if (isProgram) {
		glGetProgramiv(shader, flag, &success);
	}
	else {
		glGetShaderiv(shader, flag, &success);
	}

	if (success == GL_FALSE) {
		std::array<GLchar, 1024> error {};
		if (isProgram) {
			glGetProgramInfoLog(shader, error.size(), nullptr, error.data());
		}
		else {
			glGetShaderInfoLog(shader, error.size(), nullptr, error.data());
		}

		throw std::runtime_error {std::format("{}: '{}'", errorMessage, error.data())};
	}
}
