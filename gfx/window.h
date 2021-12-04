#ifndef DISPLAY_INCLUDED_H
#define DISPLAY_INCLUDED_H

#include "ptr.hpp"
#include <SDL2/SDL.h>
#include <special_members.hpp>
#include <string>
#include <type_traits>

class Window {
public:
	Window(int width, int height, const std::string & title);
	~Window() = default;

	NO_COPY(Window);
	NO_MOVE(Window);

	void Clear(float r, float g, float b, float a) const;
	void SwapBuffers() const;

private:
	using GL_Context = std::remove_pointer_t<SDL_GLContext>;
	using SDL_WindowPtr = wrapped_ptrt<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;
	using SDL_GLContextPtr = wrapped_ptrt<GL_Context, SDL_GL_CreateContext, SDL_GL_DeleteContext>;
	SDL_WindowPtr m_window;
	SDL_GLContextPtr m_glContext;
};

#endif
