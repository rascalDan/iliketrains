#pragma once

#include "programHandle.h"
#include <GL/glew.h>
#include <cstddef>
#include <glm/glm.hpp>

class UIShader {
public:
	UIShader(std::size_t width, std::size_t height);
	void useDefault() const;
	void useText(glm::vec3) const;

private:
	class UIProgramHandle : public ProgramHandleBase {
		using ProgramHandleBase::ProgramHandleBase;
	};
	UIProgramHandle progDefault, progText;
};
