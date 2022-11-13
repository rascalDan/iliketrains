#pragma once

#include "chronology.hpp"
#include "sceneRenderer.h"
#include "window.h"
#include <cstddef>

class GameMainWindow : public Window, SceneRenderer {
public:
	GameMainWindow(size_t w, size_t h);

	void tick(TickDuration) override;

	void render() const override;
};
