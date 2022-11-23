#pragma once

#include <special_members.hpp>

class SceneShader;

class Renderable {
public:
	Renderable() = default;
	virtual ~Renderable() = default;
	DEFAULT_MOVE_COPY(Renderable);

	virtual void render(const SceneShader & shader) const = 0;
	virtual void lights(const SceneShader & shader) const;
};
