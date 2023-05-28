#pragma once

#include "program.h"
#include <cstddef>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class UIShader {
public:
	UIShader(std::size_t width, std::size_t height);

private:
	explicit UIShader(const glm::mat4 & viewProjection);

	class UIProgram : public Program {
	public:
		template<typename... S>
		explicit UIProgram(const glm::mat4 & vp, S &&... srcs) : Program {std::forward<S>(srcs)...}
		{
			const RequiredUniformLocation uiProjectionLoc {*this, "uiProjection"};
			glUseProgram(*this);
			glUniformMatrix4fv(uiProjectionLoc, 1, GL_FALSE, glm::value_ptr(vp));
		}
	};

	class IconProgram : public UIProgram {
	public:
		explicit IconProgram(const glm::mat4 & vp);
		using Program::use;
	};

	class TextProgram : public UIProgram {
	public:
		explicit TextProgram(const glm::mat4 & vp);
		void use(const glm::vec3 & colour) const;

	private:
		RequiredUniformLocation colorLoc;
	};

public:
	IconProgram icon;
	TextProgram text;
};
