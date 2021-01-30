#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <special_members.hpp>

union SDL_Event;

class InputHandler {
public:
	InputHandler() = default;
	virtual ~InputHandler() = default;

	DEFAULT_MOVE_COPY(InputHandler);

	virtual bool handleInput(SDL_Event &) = 0;
};

#endif
