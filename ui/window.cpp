#include "window.h"
#include "uiComponent.h"
#include "worldOverlay.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <tuple>
#include <type_traits>

Window::GlewInitHelper::GlewInitHelper()
{
	[[maybe_unused]] static auto init = []() {
		if (const auto r = glewInit(); r != GLEW_OK) {
			throw std::runtime_error {reinterpret_cast<const char *>(glewGetErrorString(r))};
		}
		else {
			return r;
		}
	}();
}

Window::Window(size_t width, size_t height, const std::string & title, Uint32 flags) :
	size {static_cast<int>(width), static_cast<int>(height)}, m_window {title.c_str(),
																	  static_cast<int>(SDL_WINDOWPOS_CENTERED),
																	  static_cast<int>(SDL_WINDOWPOS_CENTERED), size.x,
																	  size.y, flags},
	glContext {m_window}, uiShader {width, height}
{
}

void
Window::clear(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
Window::swapBuffers() const
{
	SDL_GL_SwapWindow(m_window);
}

bool
Window::handleInput(const SDL_Event & e)
{
	if (SDL_GetWindowID(m_window) == e.window.windowID) {
		SDL_Event eAdjusted {e};
		switch (e.type) {
			// SDL and OpenGL have coordinates that are vertically opposed.
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				eAdjusted.button.y = size.y - e.button.y;
				break;
			case SDL_MOUSEMOTION:
				eAdjusted.motion.y = size.y - e.motion.y;
				break;
		}
		uiComponents.rapplyOne(&UIComponent::handleInput, eAdjusted, UIComponent::Position {{}, size});
		return true;
	}
	return false;
}

void
Window::refresh() const
{
	SDL_GL_MakeCurrent(m_window, glContext);
	clear(0.0F, 0.0F, 0.0F, 1.0F);

	render();

	swapBuffers();
}

void
Window::render() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	uiComponents.apply(&UIComponent::render, uiShader, UIComponent::Position {});
}
