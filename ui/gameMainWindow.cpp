#include "gameMainWindow.h"
#include "editNetwork.h"
#include "gameMainSelector.h"
#include "imgui_extras.h"
#include "manualCameraController.h"
#include "queryTool.h"
#include "svgIcon.h"
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

class GameMainToolbar : public UIComponent {
public:
	static constexpr auto TOOLBAR_HEIGHT = 54.F;
	template<typename T> static constexpr T TOOLBAR_ICON_SIZE {32, 32};

	explicit GameMainToolbar(GameMainSelector * gms) : gms {gms} { }

	void
	render() override
	{
		if (IltGui::BeginToolbar("bottomBar", ImGuiDir_Down, TOOLBAR_HEIGHT)) {
			if (ImGui::ImageButton("Build rails", *buildRailsIcon, TOOLBAR_ICON_SIZE<ImVec2>)) {
				gms->target = std::make_unique<EditNetworkOf<RailLinks>>();
			}
			if (ImGui::ImageButton("Query", *queryToolIcon, TOOLBAR_ICON_SIZE<ImVec2>)) {
				gms->target = std::make_unique<QueryTool>();
			}
			IltGui::EndToolbar();
		}
	}

	bool
	handleInput(const SDL_Event &) override
	{
		return false;
	}

private:
	SvgIcon buildRailsIcon {TOOLBAR_ICON_SIZE<ImageDimensions>, "ui/icon/rails.svg"};
	SvgIcon queryToolIcon {TOOLBAR_ICON_SIZE<ImageDimensions>, "ui/icon/magnifier.svg"};
	GameMainSelector * gms;
};

GameMainWindow::GameMainWindow(size_t w, size_t h) : SceneRenderer {{w, h}, 0}
{
	uiComponents.create<ManualCameraController>(glm::vec2 {310'727'624, 494'018'810});
	auto gms = uiComponents.create<GameMainSelector>(&camera);
	uiComponents.create<GameMainToolbar>(gms.get());
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
	uiComponents.apply(&UIComponent::render);
}

void
GameMainWindow::content(const SceneShader & shader, const Frustum & frustum) const
{
	for (const auto & [id, asset] : gameState->assets) {
		if (const auto r = std::dynamic_pointer_cast<const Renderable>(asset)) {
			r->render(shader, frustum);
		}
	}
	gameState->world.apply<Renderable>(&Renderable::render, shader, frustum);
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
	gameState->world.apply<Renderable>(&Renderable::lights, shader);
}

void
GameMainWindow::shadows(const ShadowMapper & shadowMapper, const Frustum & frustum) const
{
	for (const auto & [id, asset] : gameState->assets) {
		if (const auto r = std::dynamic_pointer_cast<const Renderable>(asset)) {
			r->shadows(shadowMapper, frustum);
		}
	}
	gameState->world.apply<Renderable>(&Renderable::shadows, shadowMapper, frustum);
}
