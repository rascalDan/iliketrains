#include "gameMainSelector.h"
#include "collection.hpp"
#include "text.h"
#include "ui/uiComponent.h"
#include <SDL2/SDL.h>
#include <game/gamestate.h>
#include <game/geoData.h>
#include <game/selectable.h>
#include <game/worldobject.h> // IWYU pragma: keep
#include <gfx/gl/camera.h>
#include <optional>
#include <span>
#include <stream_support.hpp>
#include <typeinfo>
#include <vector>

GameMainSelector::GameMainSelector(const Camera * c, glm::vec2 size) : UIComponent {{{}, size}}, camera {c} { }

void
GameMainSelector::render(const UIShader & shader, const Position & pos) const
{
	if (!clicked.empty()) {
		Text {clicked, {{50, 10}, {0, 15}}, {1, 1, 0}}.render(shader, pos);
	}
}

bool
GameMainSelector::handleInput(const SDL_Event & e, const Position &)
{
	switch (e.type) {
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.button == SDL_BUTTON_LEFT) {
				const auto mouse = glm::vec2 {e.button.x, e.button.y} / position.size;
				const auto ray = camera->unProject(mouse);

				if (target) {
					target->click(ray);
				}
				else {
					defaultClick(ray);
				}
				return true;
			}
			break;

		case SDL_MOUSEMOTION:
			if (target && target->handleMove()) {
				const auto mouse = glm::vec2 {e.motion.x, e.motion.y} / position.size;
				const auto ray = camera->unProject(mouse);

				if (target) {
					target->move(ray);
				}
				return true;
			}
			break;
	}
	return false;
}

void
GameMainSelector::defaultClick(const Ray & ray)
{
	glm::vec2 baryPos {};
	float distance;

	if (const auto selected
			= gameState->world.applyOne<Selectable>(&Selectable::intersectRay, ray, &baryPos, &distance);
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
