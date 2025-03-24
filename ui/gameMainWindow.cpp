#include "gameMainWindow.h"
#include "editNetwork.h"
#include "gameMainSelector.h"
#include "manualCameraController.h"
#include "modeHelper.h"
#include "toolbar.h"
#include "window.h"
#include <SDL2/SDL.h>
#include <collection.h>
#include <game/environment.h>
#include <game/gamestate.h>
#include <game/network/rail.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/camera_controller.h>
#include <gfx/renderable.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

class GameMainToolbar : Mode<decltype(GameMainSelector::target)>, public Toolbar {
public:
	explicit GameMainToolbar(GameMainSelector * gms_) :
		Mode<decltype(GameMainSelector::target)> {gms_->target},
		Toolbar {
				{"ui/icon/network.png", toggle<EditNetworkOf<RailLinks>>()},
		}
	{
	}
};

GameMainWindow::GameMainWindow(size_t w, size_t h) : WindowContent {w, h}, SceneRenderer {{w, h}, 0}
{
	uiComponents.create<ManualCameraController>(glm::vec2 {310'727'624, 494'018'810});
	auto gms = uiComponents.create<GameMainSelector>(&camera, ScreenAbsCoord {w, h});
	uiComponents.create<GameMainToolbar>(gms);
}

GameMainWindow::~GameMainWindow() { }

void
GameMainWindow::tick(TickDuration)
{
	uiComponents.apply<CameraController>(&CameraController::updateCamera, &camera);
}

bool
GameMainWindow::handleInput(const SDL_Event & event)
{
	switch (event.type) {
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					SceneRenderer::resize({event.window.data1, event.window.data2});
					break;
				default:
					break;
			}
		default:
			break;
	}

	return WindowContent::handleInput(event);
}

void
GameMainWindow::render() const
{
	SceneRenderer::render(*this);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	uiComponents.apply(&UIComponent::render, uiShader, UIComponent::Position {});
}

void
GameMainWindow::content(const SceneShader & shader, const Frustum & frustum) const
{
	for (const auto & [assetId, asset] : gameState->assets) {
		if (const auto renderable = asset.getAs<const Renderable>()) {
			renderable->render(shader, frustum);
		}
	}
	gameState->world.apply<const Renderable>(&Renderable::render, shader, frustum);
	uiComponents.apply<WorldOverlay>(&WorldOverlay::render, shader, frustum);
}

void
GameMainWindow::environment(const SceneShader &, const SceneRenderer & r) const
{
	gameState->environment->render(r, *this);
}

void
GameMainWindow::lights(const SceneShader & shader) const
{
	gameState->world.apply<const Renderable>(&Renderable::lights, shader);
}

void
GameMainWindow::shadows(const ShadowMapper & shadowMapper, const Frustum & frustum) const
{
	for (const auto & [assetId, asset] : gameState->assets) {
		if (const auto renderable = asset.getAs<const Renderable>()) {
			renderable->shadows(shadowMapper, frustum);
		}
	}
	gameState->world.apply<const Renderable>(&Renderable::shadows, shadowMapper, frustum);
}
