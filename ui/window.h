#ifndef DISPLAY_INCLUDED_H
#define DISPLAY_INCLUDED_H

#include "chronology.hpp"
#include "collection.hpp"
#include "inputHandler.h" // IWYU pragma: keep
#include "ptr.hpp"
#include <SDL2/SDL.h>
#include <special_members.hpp>
#include <string>
#include <type_traits>

class GameState;

class Window {
public:
	Window(int width, int height, const std::string & title);
	virtual ~Window() = default;

	NO_COPY(Window);
	NO_MOVE(Window);

	virtual void tick(TickDuration elapsed) = 0;
	void refresh(const GameState *) const;
	bool handleInput(const SDL_Event & e);

	void clear(float r, float g, float b, float a) const;
	void swapBuffers() const;

protected:
	SDL_GLContext glContext() const;
	virtual void render(const GameState *) const = 0;

	using SDL_WindowPtr = wrapped_ptrt<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;
	SDL_WindowPtr m_window;
	Collection<InputHandler> inputStack;
};

#endif
