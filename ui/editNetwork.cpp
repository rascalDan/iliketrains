#include "editNetwork.h"
#include "builders/straight.h"
#include "text.h"
#include <game/gamestate.h>
#include <game/geoData.h>

EditNetwork::EditNetwork(Network * n) :
	network {n}, builderToolbar {
						 {"ui/icon/network.png", mode.toggle<BuilderStraight>()},
				 }
{
}

EditNetwork::~EditNetwork() = default;

bool
EditNetwork::click(const SDL_MouseButtonEvent & e, const Ray & ray)
{
	if (builder) {
		builder->click(network, gameState->geoData.get(), e, ray);
		return true;
	}
	return false;
}

bool
EditNetwork::move(const SDL_MouseMotionEvent &, const Ray &)
{
	return false;
}

bool
EditNetwork::handleInput(const SDL_Event & e, const UIComponent::Position & parentPos)
{
	return builderToolbar.handleInput(e, parentPos);
}

void
EditNetwork::render(const Shader & shader) const
{
	if (builder) {
		builder->render(shader);
	}
}

void
EditNetwork::render(const UIShader & shader, const UIComponent::Position & parentPos) const
{
	if (builder) {
		Text {builder->hint(), {{50, 10}, {0, 15}}, {1, 1, 0}}.render(shader, parentPos);
	}
	builderToolbar.render(shader, parentPos);
}
