#pragma once

#include "config/types.h"
#include "gl_traits.h"
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
			glUniform(uiProjectionLoc, vp);
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
		void use(const RGB & colour) const;

	private:
		RequiredUniformLocation colorLoc;
	};

public:
	IconProgram icon;
	TextProgram text;
};
