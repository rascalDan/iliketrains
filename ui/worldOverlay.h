#pragma once

class SceneShader;
class Frustum;

class WorldOverlay {
public:
	virtual ~WorldOverlay() = default;
	virtual void render(const SceneShader &, const Frustum &) const = 0;
};
