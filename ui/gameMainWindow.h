#pragma once

#include "chronology.h"
#include "gfx/gl/sceneRenderer.h"
#include "windowContent.h"

class GameMainWindow : public WindowContent, SceneRenderer, public SceneProvider {
public:
	GameMainWindow(ScreenAbsCoord size);

	void tick(TickDuration) override;
	void render() override;

private:
	bool handleInput(const SDL_Event &) override;

	void content(const SceneShader &, const Frustum &) const override;
	void environment(const SceneShader &, const SceneRenderer &) const override;
	void lights(const SceneShader &) const override;
	void shadows(const ShadowMapper &, const Frustum &) const override;
};
