#pragma once

class SceneRenderer;
class ShadowMapper;
class SceneShader;

class SceneProvider {
public:
	virtual ~SceneProvider() = default;
	virtual void content(const SceneShader &) const = 0;
	virtual void environment(const SceneShader &, const SceneRenderer &) const;
	virtual void lights(const SceneShader &) const = 0;
	virtual void shadows(const ShadowMapper &) const;
};
