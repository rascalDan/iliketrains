#include "window.h"
#include "ui/inputHandler.h"
#include <GL/glew.h>
#include <optional>
#include <stdexcept>

static SDL_GLContext
SDL_GL_CreateContextAndGlewInit(SDL_Window * w)
{
	auto ctx = SDL_GL_CreateContext(w);
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error {"Glew failed to initialize!"};
	}
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	return ctx;
}

using GL_Context = std::remove_pointer_t<SDL_GLContext>;
using SDL_GLContextPtr = wrapped_ptrt<GL_Context, SDL_GL_CreateContextAndGlewInit, SDL_GL_DeleteContext>;

Window::Window(int width, int height, const std::string & title) :
	m_window {title.c_str(), static_cast<int>(SDL_WINDOWPOS_CENTERED), static_cast<int>(SDL_WINDOWPOS_CENTERED), width,
			height, static_cast<Uint32>(SDL_WINDOW_OPENGL)}
{
	glContext();
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
		inputStack.applyOne(&InputHandler::handleInput, e);
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
