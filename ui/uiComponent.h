#pragma once

#include <glm/glm.hpp>
#include <special_members.h>

union SDL_Event;

class UIComponent {
public:
	UIComponent() = default;
	virtual ~UIComponent() = default;

	NO_MOVE(UIComponent);
	NO_COPY(UIComponent);

	virtual void render() const = 0;
	virtual bool handleInput(const SDL_Event &) = 0;
};
