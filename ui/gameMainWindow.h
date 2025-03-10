#pragma once

#include "chronology.h"
#include "gfx/gl/sceneRenderer.h"
#include "windowContent.h"
#include <cstddef>

class GameMainWindow : public WindowContent, SceneRenderer, public SceneProvider {
public:
	GameMainWindow(size_t w, size_t h);
	~GameMainWindow() override;

	NO_MOVE(GameMainWindow);
	NO_COPY(GameMainWindow);

	void tick(TickDuration) override;
	void render() const override;

private:
	void content(const SceneShader &, const Frustum &) const override;
	void environment(const SceneShader &, const SceneRenderer &) const override;
	void lights(const SceneShader &) const override;
	void shadows(const ShadowMapper &, const Frustum &) const override;
};
