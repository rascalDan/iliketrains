#include "window.h"
#include <GL/glew.h>
#include <stdexcept>

Window::Window(int width, int height, const std::string & title) :
	m_window {m_window.create(SDL_CreateWindow, SDL_DestroyWindow, title.c_str(), SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL)},
	m_glContext {m_glContext.create(SDL_GL_CreateContext, SDL_GL_DeleteContext, m_window)}
{
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error {"Glew failed to initialize!"};
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
Window::Clear(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
Window::SwapBuffers() const
{
	SDL_GL_SwapWindow(m_window);
}
