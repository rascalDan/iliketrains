#include "gameMainWindow.h"
#include "editNetwork.h"
#include "gameMainSelector.h"
#include "gfx/camera_controller.h"
#include "manualCameraController.h"
#include "maths.h"
#include "modeHelper.hpp"
#include "toolbar.h"
#include "window.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <collection.hpp>
#include <game/gamestate.h>
#include <game/network/rail.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <utility>

class GameMainToolbar : Mode<decltype(GameMainSelector::target)>, public Toolbar {
public:
	explicit GameMainToolbar(GameMainSelector * gms_) :
		Mode<decltype(GameMainSelector::target)> {gms_->target}, Toolbar {
																		 {"ui/icon/network.png",
																				 toggle<EditNetworkOf<RailLinks>>()},
																 }
	{
	}
};

GameMainWindow::GameMainWindow(size_t w, size_t h) :
	Window {w, h, "I Like Trains"}, SceneRenderer {size}, camera {{-1250.0F, -1250.0F, 35.0F}, quarter_pi, rdiv(w, h),
																  0.1F, 10000.0F}
{
	uiComponents.create<ManualCameraController>(glm::vec2 {-1150, -1150});
	auto gms = uiComponents.create<GameMainSelector>(&camera, glm::vec2 {w, h});
	uiComponents.create<GameMainToolbar>(gms.get());
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
	SceneRenderer::render([this] {
		gameState->world.apply<Renderable>(&Renderable::render, shader);
		uiComponents.apply<WorldOverlay>(&WorldOverlay::render, shader);
	});
	Window::render();
}
