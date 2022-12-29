#include "testMainWindow.h"
#include <boost/test/test_tools.hpp>

TestMainWindow::TestMainWindow() : Window {1, 1, __FILE__, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN}
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
			[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message,
					const void *) {
				char buf[BUFSIZ];
				snprintf(buf, BUFSIZ, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
						(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
				switch (type) {
					case GL_DEBUG_TYPE_ERROR:
					case GL_DEBUG_TYPE_PERFORMANCE:
					case GL_DEBUG_TYPE_PORTABILITY:
					case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
						BOOST_TEST_ERROR(buf);
				}
				BOOST_TEST_MESSAGE(buf);
			},
			nullptr);
}
