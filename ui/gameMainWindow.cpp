#include "gameMainWindow.h"
#include "editNetwork.h"
#include "gameMainSelector.h"
#include "gfx/camera_controller.h"
#include "manualCameraController.h"
#include "maths.h"
#include "toolbar.h"
#include "window.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <collection.hpp>
#include <game/gamestate.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <utility>

class GameMainToolbar : public Toolbar {
public:
	explicit GameMainToolbar(GameMainSelector * gms_) :
		Toolbar {
				{"ui/icon/network.png",
						[this](const SDL_Event &) {
							toggleSetMode<EditNetwork>();
						}},
		},
		gms {gms_}
	{
	}

private:
	template<typename UiMode, typename... Params>
	void
	toggleSetMode(Params &&... params)
	{
		if (dynamic_cast<UiMode *>(gms->target.get())) {
			gms->target.reset();
		}
		else {
			gms->target = std::make_unique<UiMode>(std::forward<Params>(params)...);
		}
	}

	GameMainSelector * gms;
};

GameMainWindow::GameMainWindow(size_t w, size_t h) :
	Window {w, h, "I Like Trains"}, camera {{-1250.0F, -1250.0F, 35.0F}, quarter_pi, rdiv(w, h), 0.1F, 10000.0F}
{
	uiComponents.create<ManualCameraController>(glm::vec2 {-1150, -1150});
	auto gms = uiComponents.create<GameMainSelector>(&camera, glm::vec2 {w, h});
	uiComponents.create<GameMainToolbar>(gms.get());

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

const Shader &
GameMainWindow::getShader() const
{
	return shader;
}
