#include "toolbar.h"
#include "ui/iconButton.h"
#include "ui/uiComponent.h"
#include "uiComponentPlacer.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

Toolbar::Toolbar(const std::initializer_list<InitInfo> & initInfo) : UIComponent {{{}, {}}}
{
	UIComponentPlacer placer {{10, 10}, 5, 1};
	for (const auto & ii : initInfo) {
		icons.create(ii.first, placer.next(ICON_SIZE), ii.second);
	}
	this->position.size = placer.getLimit();
}

void
Toolbar::render(const UIShader & uiShader, const Position & parentPos) const
{
	const auto absPos = this->position + parentPos;
	icons.apply(&UIComponent::render, uiShader, absPos);
}

bool
Toolbar::handleInput(const SDL_Event & e, const Position & parentPos)
{
	const auto absPos = this->position + parentPos;
	if (absPos & e.button) {
		icons.applyOne(&UIComponent::handleInput, e, absPos);
		return true;
	}
	return false;
}
