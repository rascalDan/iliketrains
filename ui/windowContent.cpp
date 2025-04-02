#include "windowContent.h"
#include "SDL_events.h"

void
WindowContent::tick(TickDuration)
{
}

bool
WindowContent::handleInput(const SDL_Event & e)
{
	SDL_Event eAdjusted {e};
	const auto size = [&e] {
		glm::ivec2 size {};
		SDL_GetWindowSizeInPixels(SDL_GetWindowFromID(e.window.windowID), &size.x, &size.y);
		return size;
	}();
	switch (e.type) {
			//  SDL and OpenGL have coordinates that are vertically opposed.
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			eAdjusted.button.y = size.y - e.button.y;
			break;
		case SDL_MOUSEMOTION:
			eAdjusted.motion.y = size.y - e.motion.y;
			break;
	}
	uiComponents.rapplyOne(&UIComponent::handleInput, eAdjusted);
	return true;
}
