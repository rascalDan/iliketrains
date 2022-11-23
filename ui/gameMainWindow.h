#pragma once

#include "chronology.hpp"
#include "sceneRenderer.h"
#include "window.h"
#include <cstddef>

class GameMainWindow : public Window, SceneRenderer, public SceneRenderer::SceneProvider {
public:
	GameMainWindow(size_t w, size_t h);

	void tick(TickDuration) override;

	void render() const override;

private:
	void content(const SceneShader &) const override;
	void environment(const SceneShader &, const SceneRenderer &) const override;
	void lights(const SceneShader &) const override;
};
