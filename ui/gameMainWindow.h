#ifndef UI_GAMEMAINWINDOW_H
#define UI_GAMEMAINWINDOW_H

#include "chronology.hpp"
#include "gfx/gl/camera.h"
#include "gfx/gl/shader.h"
#include "window.h"
#include <cstddef>

class GameState;

class GameMainWindow : public Window {
public:
	GameMainWindow(size_t w, size_t h, const GameState *);

	void tick(TickDuration) override;

	void render(const GameState * gameState) const override;

private:
	Shader shader;
	Camera camera;
};

#endif