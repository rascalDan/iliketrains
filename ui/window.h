#pragma once

#include "chronology.h"
#include "collection.h"
#include "gfx/gl/uiShader.h"
#include "ptr.h"
#include "uiComponent.h" // IWYU pragma: keep
#include <SDL2/SDL.h>
#include <cstddef>
#include <special_members.h>
#include <string>

using SDL_WindowPtr = wrapped_ptrt<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;
using GL_Context = std::remove_pointer_t<SDL_GLContext>;
using SDL_GLContextPtr = wrapped_ptrt<GL_Context, SDL_GL_CreateContext, SDL_GL_DeleteContext>;

class Window {
public:
	Window(size_t width, size_t height, const std::string & title, Uint32 flags);
	virtual ~Window() = default;

	NO_COPY(Window);
	NO_MOVE(Window);

	virtual void tick(TickDuration elapsed) = 0;
	void refresh() const;
	bool handleInput(const SDL_Event & e);

	void clear(float r, float g, float b, float a) const;
	void swapBuffers() const;

protected:
	virtual void render() const;

	struct GLInitHelper {
		GLInitHelper();
	};

	const ScreenAbsCoord size;
	SDL_WindowPtr m_window;
	SDL_GLContextPtr glContext;
	GLInitHelper glInithelper;
	Collection<UIComponent> uiComponents;
	UIShader uiShader;
};
