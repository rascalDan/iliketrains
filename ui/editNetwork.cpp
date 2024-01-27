#include "editNetwork.h"
#include "builders/freeExtend.h"
#include "builders/join.h"
#include "builders/straight.h"
#include "text.h"
#include <game/gamestate.h>
#include <game/geoData.h>
#include <gfx/gl/sceneShader.h>
#include <gfx/models/texture.h>

const std::filesystem::path fontpath {"/usr/share/fonts/hack/Hack-Regular.ttf"};
constexpr const glm::u8vec4 TRANSPARENT_BLUE {30, 50, 255, 200};

EditNetwork::EditNetwork(Network * n) :
	network {n},
	builderToolbar {
			{"ui/icon/network.png", mode.toggle<BuilderStraight>()},
			{"ui/icon/network.png", mode.toggle<BuilderJoin>()},
			{"ui/icon/network.png", mode.toggle<BuilderFreeExtend>()},
	},
	blue {1, 1, &TRANSPARENT_BLUE}, font {fontpath, 15}
{
}

bool
EditNetwork::click(const SDL_MouseButtonEvent & e, const Ray<GlobalPosition3D> & ray)
{
	if (builder && (e.button == SDL_BUTTON_LEFT || e.button == SDL_BUTTON_MIDDLE)) {
		builder->click(network, gameState->geoData.get(), e, ray);
		return true;
	}
	return false;
}

bool
EditNetwork::move(const SDL_MouseMotionEvent & e, const Ray<GlobalPosition3D> & ray)
{
	if (builder) {
		builder->move(network, gameState->geoData.get(), e, ray);
	}
	return false;
}

bool
EditNetwork::handleInput(const SDL_Event & e, const UIComponent::Position & parentPos)
{
	return builderToolbar.handleInput(e, parentPos);
}

void
EditNetwork::render(const SceneShader & shader) const
{
	if (builder) {
		blue.bind();
		shader.absolute.use();
		builder->render(shader);
	}
}

void
EditNetwork::Builder::render(const SceneShader & shader) const
{
	candidateLinks.apply<const Renderable>(&Renderable::render, shader);
}

void
EditNetwork::render(const UIShader & shader, const UIComponent::Position & parentPos) const
{
	if (builder) {
		Text {builder->hint(), font, {{50, 10}, {0, 15}}, {1, 1, 0}}.render(shader, parentPos);
	}
	builderToolbar.render(shader, parentPos);
}
