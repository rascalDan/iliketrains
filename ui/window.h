#pragma once

#include "chronology.hpp"
#include "collection.hpp"
#include "gfx/gl/uiShader.h"
#include "ptr.hpp"
#include "uiComponent.h" // IWYU pragma: keep
#include <SDL2/SDL.h>
#include <cstddef>
#include <special_members.hpp>
#include <string>

class Shader;

class Window {
public:
	Window(size_t width, size_t height, const std::string & title);
	virtual ~Window() = default;

	NO_COPY(Window);
	NO_MOVE(Window);

	virtual void tick(TickDuration elapsed) = 0;
	void refresh() const;
	bool handleInput(const SDL_Event & e);

	void clear(float r, float g, float b, float a) const;
	void swapBuffers() const;

protected:
	[[nodiscard]] SDL_GLContext glContext() const;
	virtual void render() const;
	virtual const Shader & getShader() const = 0;

	using SDL_WindowPtr = wrapped_ptrt<SDL_Window, SDL_CreateWindow, SDL_DestroyWindow>;
	SDL_WindowPtr m_window;
	Collection<UIComponent> uiComponents;
	UIShader uiShader;
};
