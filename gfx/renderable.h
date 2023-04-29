#pragma once

#include <special_members.h>

class SceneShader;
class ShadowMapper;

class Renderable {
public:
	Renderable() = default;
	virtual ~Renderable() = default;
	DEFAULT_MOVE_COPY(Renderable);

	virtual void render(const SceneShader & shader) const = 0;
	virtual void lights(const SceneShader & shader) const;
	virtual void shadows(const ShadowMapper & shadowMapper) const;
};
