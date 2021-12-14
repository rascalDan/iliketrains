#include "gameMainWindow.h"
#include "collection.hpp"
#include "gfx/camera_controller.h"
#include "manualCameraController.h"
#include "ui/window.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <game/gamestate.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <maths.h>
#include <memory>

GameMainWindow::GameMainWindow(size_t w, size_t h) :
	Window {static_cast<int>(w), static_cast<int>(h), "I Like Trains"}, uiShader {w, h}, camera {{-1250.0F, -1250.0F,
																										 35.0F},
																								 70.0F, rdiv(w, h),
																								 0.1F, 10000.0F}
{
	inputStack.create<ManualCameraController>(glm::vec2 {-1150, -1150});

	shader.setUniform("lightDirection", glm::normalize(glm::vec3 {1, 0, -1}));
	shader.setUniform("lightColor", {.6, .6, .6});
	shader.setUniform("ambientColor", {0.5, 0.5, 0.5});
}

void
GameMainWindow::tick(TickDuration)
{
	inputStack.apply<CameraController>(&CameraController::updateCamera, &camera);
	shader.setView(camera.GetViewProjection());
}

void
GameMainWindow::render(const GameState * gameState) const
{
	glEnable(GL_DEPTH_TEST);
	gameState->world.apply<Renderable>(&Renderable::render, shader);
	glDisable(GL_DEPTH_TEST);
	// do ui bits
}
