#include "gameMainSelector.h"
#include "ui/uiComponent.h"
#include <SDL2/SDL.h>
#include <game/gamestate.h>
#include <game/selectable.h>
#include <game/terrain.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/camera.h>
#include <stream_support.h>

GameMainSelector::GameMainSelector(const Camera * c) : camera {c} { }

constexpr ScreenAbsCoord TargetPos {5, 45};

void
GameMainSelector::render() const
{
	if (target) {
		target->render();
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
GameMainSelector::handleInput(const SDL_Event & e)
{
	const auto getRay = [this, &window = e.window](const auto & e) {
		glm::ivec2 size {};
		SDL_GetWindowSizeInPixels(SDL_GetWindowFromID(window.windowID), &size.x, &size.y);
		const auto mouse = ScreenRelCoord {e.x, e.y} / ScreenRelCoord {size};
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
		return target->handleInput(e);
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
GameMainSelector::defaultClick(const Ray<GlobalPosition3D> &)
{
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
GameMainSelector::Component::handleInput(const SDL_Event &)
{
	return false;
}

void
GameMainSelector::Component::render()
{
}

void
GameMainSelector::Component::render(const SceneShader &, const Frustum &) const
{
}
