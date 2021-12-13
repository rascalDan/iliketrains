#ifndef UI_GAMEMAINWINDOW_H
#define UI_GAMEMAINWINDOW_H

#include "chronology.hpp"
#include "gfx/gl/camera.h"
#include "gfx/gl/shader.h"
#include "gfx/gl/uiShader.h"
#include "window.h"
#include <cstddef>

class GameState;

class GameMainWindow : public Window {
public:
	GameMainWindow(size_t w, size_t h);

	void tick(TickDuration) override;

	void Refresh(const GameState * gameState) const override;

private:
	UIShader uiShader;
	Shader shader;
	Camera camera;
};

#endif
