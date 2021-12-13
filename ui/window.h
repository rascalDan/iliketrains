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

	void Clear(float r, float g, float b, float a) const;
	void SwapBuffers() const;
	virtual void tick(TickDuration elapsed) = 0;
	virtual void Refresh(const GameState *) const = 0;
	bool handleInput(const SDL_Event & e);

protected:
	using GL_Context = std::remove_pointer_t<SDL_GLContext>;
	using SDL_WindowPtr = wrapped_ptrt<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;
	using SDL_GLContextPtr = wrapped_ptrt<GL_Context, SDL_GL_CreateContext, SDL_GL_DeleteContext>;
	SDL_WindowPtr m_window;
	SDL_GLContextPtr m_glContext;
	Collection<InputHandler> inputStack;
};

#endif
