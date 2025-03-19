#include "gameMainSelector.h"
#include "collection.h"
#include "text.h"
#include "ui/uiComponent.h"
#include <SDL2/SDL.h>
#include <game/gamestate.h>
#include <game/selectable.h>
#include <game/terrain.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/camera.h>
#include <optional>
#include <stream_support.h>
#include <typeinfo>

const std::filesystem::path fontpath {"/usr/share/fonts/hack/Hack-Regular.ttf"};

GameMainSelector::GameMainSelector(const Camera * c, ScreenAbsCoord size) :
	UIComponent {{{}, size}}, camera {c}, font {fontpath, 15}
{
}

constexpr ScreenAbsCoord TargetPos {5, 45};

void
GameMainSelector::render(const UIShader & shader, const Position & parentPos) const
{
	if (target) {
		target->render(shader, parentPos + position + TargetPos);
	}
	if (!clicked.empty()) {
		Text {clicked, font, {{50, 10}, {0, 15}}, {1, 1, 0}}.render(shader, parentPos);
	}
}

void
GameMainSelector::render(const SceneShader & shader, const Frustum & frustum) const
{
	if (target) {
		target->render(shader, frustum);
	}
}

bool
GameMainSelector::handleInput(const SDL_Event & e, const Position & parentPos)
{
	const auto getRay = [this](const auto & e) {
		const auto mouse = ScreenRelCoord {e.x, e.y} / position.size;
		return camera->unProject(mouse);
	};
	if (target) {
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN:
				if (target->click(e.button, getRay(e.button))) {
					return true;
				}
				break;
			case SDL_MOUSEMOTION:
				if (target->move(e.motion, getRay(e.motion))) {
					return true;
				}
				break;
		}
		return target->handleInput(e, parentPos + position + TargetPos);
	}
	else {
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN:
				defaultClick(getRay(e.button));
				break;
		}
	}
	return false;
}

void
GameMainSelector::defaultClick(const Ray<GlobalPosition3D> & ray)
{
	BaryPosition baryPos {};
	RelativeDistance distance {};

	if (const auto selected = gameState->world.applyOne<Selectable>(&Selectable::intersectRay, ray, baryPos, distance);
			selected != gameState->world.end()) {
		const auto & ref = *selected.base()->get();
		clicked = typeid(ref).name();
	}
	else if (const auto pos = gameState->terrain->intersectRay(ray)) {
		clicked = streamed_string(*pos);
	}
	else {
		clicked.clear();
	}
}

bool
GameMainSelector::Component::click(const SDL_MouseButtonEvent &, const Ray<GlobalPosition3D> &)
{
	return false;
}

bool
GameMainSelector::Component::move(const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> &)
{
	return false;
}

bool
GameMainSelector::Component::handleInput(const SDL_Event &, const Position &)
{
	return false;
}

void
GameMainSelector::Component::render(const UIShader &, const UIComponent::Position &)
{
}

void
GameMainSelector::Component::render(const SceneShader &, const Frustum &) const
{
}
