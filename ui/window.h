#pragma once

#include "chronology.h"
#include "config/types.h"
#include "ptr.h"
#include "special_members.h"
#include "windowContent.h"
#include <SDL2/SDL.h>
#include <cstddef>
#include <string>

using SDL_WindowPtr = wrapped_ptrt<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;
using GL_Context = std::remove_pointer_t<SDL_GLContext>;
using SDL_GLContextPtr = wrapped_ptrt<GL_Context, SDL_GL_CreateContext, SDL_GL_DeleteContext>;

class Window {
public:
	Window(size_t width, size_t height, const char * title, Uint32 flags);
	virtual ~Window() = default;

	NO_COPY(Window);
	NO_MOVE(Window);

	template<typename C, typename... P>
	void
	setContent(P &&... p)
	{
		glm::ivec2 size {};
		SDL_GetWindowSizeInPixels(m_window, &size.x, &size.y);
		content = std::make_unique<C>(size.x, size.y, std::forward<P>(p)...);
	}

	void tick(TickDuration elapsed);
	void refresh() const;
	bool handleInput(const SDL_Event & e);

	void swapBuffers() const;

protected:
	void clear(float r, float g, float b, float a) const;

	const ScreenAbsCoord size;
	SDL_WindowPtr m_window;
	SDL_GLContextPtr glContext;
	WindowContent::Ptr content;
};
