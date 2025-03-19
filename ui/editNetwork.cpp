#include "editNetwork.h"
#include "builders/freeExtend.h"
#include "builders/join.h"
#include "builders/straight.h"
#include "imgui_wrap.h"
#include "text.h"
#include <game/gamestate.h>
#include <game/terrain.h>
#include <gfx/gl/sceneShader.h>
#include <gfx/models/texture.h>

const std::filesystem::path fontpath {"/usr/share/fonts/hack/Hack-Regular.ttf"};
constexpr const glm::u8vec4 TRANSPARENT_BLUE {30, 50, 255, 200};

EditNetwork::EditNetwork(Network * n) : network {n}, blue {1, 1, &TRANSPARENT_BLUE}, font {fontpath, 15} { }

bool
EditNetwork::click(const SDL_MouseButtonEvent & e, const Ray<GlobalPosition3D> & ray)
{
	if (builder && (e.button == SDL_BUTTON_LEFT || e.button == SDL_BUTTON_MIDDLE)) {
		builder->click(network, gameState->terrain.get(), e, ray);
		return true;
	}
	return false;
}

bool
EditNetwork::move(const SDL_MouseMotionEvent & e, const Ray<GlobalPosition3D> & ray)
{
	if (builder) {
		builder->move(network, gameState->terrain.get(), e, ray);
	}
	return false;
}

bool
EditNetwork::handleInput(const SDL_Event &, const UIComponent::Position &)
{
	return false;
}

void
EditNetwork::render(const SceneShader & shader, const Frustum & frustum) const
{
	if (builder) {
		blue.bind();
		shader.absolute.use();
		builder->render(shader, frustum);
	}
}

void
EditNetwork::Builder::render(const SceneShader & shader, const Frustum & frustum) const
{
	candidateLinks.apply<const Renderable>(&Renderable::render, shader, frustum);
}

void
EditNetwork::Builder::setHeightsFor(Network * network, const Link::CCollection & links, GeoData::SetHeightsOpts opts)
{
	opts.surface = network->getBaseSurface();
	const auto width = network->getBaseWidth();

	for (const auto & link : links) {
		gameState->terrain->setHeights(link->getBase(width), opts);
	}
}

void
EditNetwork::render(const UIShader &, const UIComponent::Position &)
{
	ImGui::Begin("Edit Network");

	auto builderChoice = [this]<typename Impl>(const char * name) {
		if (ImGui::RadioButton(name, dynamic_cast<Impl *>(builder.get()))) {
			builder = std::make_unique<Impl>();
		}
	};
	builderChoice.operator()<BuilderStraight>("Straight");
	builderChoice.operator()<BuilderJoin>("Join");
	builderChoice.operator()<BuilderFreeExtend>("Free Extend");
	ImGui::TextUnformatted(builder ? builder->hint().c_str() : "Select a build mode");

	ImGui::End();
}
