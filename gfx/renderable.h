#pragma once

#include <special_members.h>

class SceneShader;
class Frustum;
class ShadowMapper;
class ShadowStenciller;

class Renderable {
public:
	Renderable() = default;
	virtual ~Renderable() = default;
	DEFAULT_MOVE_COPY(Renderable);

	virtual void render(const SceneShader & shader, const Frustum &) const = 0;
	virtual void lights(const SceneShader & shader) const;
	virtual void shadows(const ShadowMapper & shadowMapper) const;

	virtual void updateStencil(const ShadowStenciller & lightDir) const;
};
