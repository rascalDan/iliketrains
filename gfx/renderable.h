#pragma once

#include <special_members.hpp>

class Shader;

class Renderable {
public:
	Renderable() = default;
	virtual ~Renderable() = default;
	DEFAULT_MOVE_COPY(Renderable);

	virtual void render(const Shader & shader) const = 0;
};
