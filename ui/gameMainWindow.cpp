#include "gameMainWindow.h"
#include "gfx/camera_controller.h"
#include "manualCameraController.h"
#include "maths.h"
#include "toolbar.h"
#include "ui/uiComponent.h"
#include "window.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <collection.hpp>
#include <game/gamestate.h>
#include <game/selectable.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <tuple>

class UIShader;

class GameMainToolbar : public Toolbar {
public:
	GameMainToolbar() :
		Toolbar {
				{"ui/icon/network.png",
						[](const SDL_Event &) {
							// fprintf(stderr, "network click\n");
						}},
		}
	{
	}
};

class GameMainSelector : public UIComponent {
public:
	GameMainSelector(const Camera * c, glm::vec2 size) : UIComponent {{{}, size}}, camera {c} { }

	void
	render(const UIShader &, const Position &) const override
	{
	}

	bool
	handleInput(const SDL_Event & e, const Position &) override
	{
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN:
				if (e.button.button == SDL_BUTTON_LEFT) {
					const auto mouse = glm::vec2 {e.button.x, e.button.y} / position.size;

					glm::vec2 baryPos {};
					float eh;

					std::ignore = gameState->world.applyOne<Selectable>(
							&Selectable::intersectRay, camera->pos, camera->unProject(mouse), &baryPos, &eh);
				}
		}
		return false;
	}

private:
	const Camera * camera;
};

GameMainWindow::GameMainWindow(size_t w, size_t h) :
	Window {w, h, "I Like Trains"}, camera {{-1250.0F, -1250.0F, 35.0F}, quarter_pi, rdiv(w, h), 0.1F, 10000.0F}
{
	uiComponents.create<GameMainSelector>(&camera, glm::vec2 {w, h});
	uiComponents.create<GameMainToolbar>();
	uiComponents.create<ManualCameraController>(glm::vec2 {-1150, -1150});

	shader.setUniform("lightDirection", glm::normalize(glm::vec3 {1, 0, -1}));
	shader.setUniform("lightColor", {.6, .6, .6});
	shader.setUniform("ambientColor", {0.5, 0.5, 0.5});
}

void
GameMainWindow::tick(TickDuration)
{
	uiComponents.apply<CameraController>(&CameraController::updateCamera, &camera);
	shader.setView(camera.GetViewProjection());
}

void
GameMainWindow::render() const
{
	glEnable(GL_DEPTH_TEST);
	gameState->world.apply<Renderable>(&Renderable::render, shader);
	Window::render();
}
