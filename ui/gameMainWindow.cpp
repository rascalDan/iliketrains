#include "gameMainWindow.h"
#include "gfx/camera_controller.h"
#include "manualCameraController.h"
#include "maths.h"
#include "ray.hpp"
#include "text.h"
#include "toolbar.h"
#include "ui/uiComponent.h"
#include "window.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <collection.hpp>
#include <game/gamestate.h>
#include <game/geoData.h>
#include <game/selectable.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

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

#include <stream_support.hpp>

class GameMainSelector : public UIComponent {
public:
	GameMainSelector(const Camera * c, glm::vec2 size) : UIComponent {{{}, size}}, camera {c} { }

	void
	render(const UIShader & shader, const Position & pos) const override
	{
		if (!clicked.empty()) {
			Text {clicked, {{50, 10}, {0, 15}}, {1, 1, 0}}.render(shader, pos);
		}
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

					const auto ray = camera->unProject(mouse);
					if (const auto selected
							= gameState->world.applyOne<Selectable>(&Selectable::intersectRay, ray, &baryPos, &eh);
							selected != gameState->world.end()) {
						const auto & ref = *selected.base()->get();
						clicked = typeid(ref).name();
					}
					else if (const auto pos = gameState->geoData->intersectRay(ray)) {
						clicked = streamed_string(*pos);
					}
					else {
						clicked.clear();
					}
				}
		}
		return false;
	}

private:
	const Camera * camera;
	std::string clicked;
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
