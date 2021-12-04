#include "window.h"
#include <GL/glew.h>
#include <stdexcept>

Window::Window(int width, int height, const std::string & title) :
	m_window {title.c_str(), static_cast<int>(SDL_WINDOWPOS_CENTERED), static_cast<int>(SDL_WINDOWPOS_CENTERED), width,
			height, static_cast<Uint32>(SDL_WINDOW_OPENGL)},
	m_glContext {m_window}
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
