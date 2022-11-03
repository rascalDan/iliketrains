#pragma once

class SceneShader;

class WorldOverlay {
public:
	virtual ~WorldOverlay() = default;
	virtual void render(const SceneShader &) const = 0;
};
