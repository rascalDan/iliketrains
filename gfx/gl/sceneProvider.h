#pragma once

#include <special_members.h>

class SceneRenderer;
class ShadowMapper;
class SceneShader;
class Frustum;

class SceneProvider {
public:
	SceneProvider() = default;
	virtual ~SceneProvider() = default;
	DEFAULT_MOVE_COPY(SceneProvider);

	virtual void content(const SceneShader &, const Frustum &) const = 0;
	virtual void environment(const SceneShader &, const SceneRenderer &) const;
	virtual void lights(const SceneShader &) const = 0;
	virtual void shadows(const ShadowMapper &, const Frustum &) const;
};
