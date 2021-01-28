#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

union SDL_Event;

class InputHandler {
public:
	virtual ~InputHandler() = default;

	virtual bool handleInput(SDL_Event &) = 0;
};

#endif
