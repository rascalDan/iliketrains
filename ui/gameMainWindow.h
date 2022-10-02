#pragma once

#include "chronology.hpp"
#include "gfx/gl/camera.h"
#include "gfx/gl/shader.h"
#include "window.h"
#include <cstddef>

class GameMainWindow : public Window {
public:
	GameMainWindow(size_t w, size_t h);

	void tick(TickDuration) override;

	void render() const override;

private:
	const Shader & getShader() const override;
	Shader shader;
	Camera camera;
};
