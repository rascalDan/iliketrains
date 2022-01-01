#include "window.h"
#include "uiComponent.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <stdexcept>
#include <tuple>
#include <type_traits>

static SDL_GLContext
SDL_GL_CreateContextAndGlewInit(SDL_Window * w)
{
	auto ctx = SDL_GL_CreateContext(w);
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error {"Glew failed to initialize!"};
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return ctx;
}

using GL_Context = std::remove_pointer_t<SDL_GLContext>;
using SDL_GLContextPtr = wrapped_ptrt<GL_Context, SDL_GL_CreateContextAndGlewInit, SDL_GL_DeleteContext>;

Window::Window(size_t width, size_t height, const std::string & title) :
	m_window {title.c_str(), static_cast<int>(SDL_WINDOWPOS_CENTERED), static_cast<int>(SDL_WINDOWPOS_CENTERED),
			static_cast<int>(width), static_cast<int>(height), static_cast<Uint32>(SDL_WINDOW_OPENGL)},
	uiShader {[this](auto w) {
				  // must call glContent before creating the shader
				  std::ignore = glContext();
				  return w;
			  }(width),
			height}
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
		glm::ivec2 size {};
		switch (e.type) {
			// SDL and OpenGL have coordinates that are vertically opposed.
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				SDL_GetWindowSize(m_window, &size.x, &size.y);
				eAdjusted.button.y = size.y - e.button.y;
				break;
		}
		uiComponents.applyOne(&UIComponent::handleInput, eAdjusted, UIComponent::Position {{}, size});
		return true;
	}
	return false;
}

SDL_GLContext
Window::glContext() const
{
	static SDL_GLContextPtr m_glContext {m_window};
	return m_glContext;
}

void
Window::refresh(const GameState * gameState) const
{
	SDL_GL_MakeCurrent(m_window, glContext());
	clear(0.0F, 0.0F, 0.0F, 1.0F);

	render(gameState);

	swapBuffers();
}

void
Window::render(const GameState *) const
{
	glDisable(GL_DEPTH_TEST);
	uiComponents.apply(&UIComponent::render, uiShader, UIComponent::Position {});
}
