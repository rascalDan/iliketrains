#include "testMainWindow.h"
#include "testHelpers.h"
#include <boost/test/test_tools.hpp>

TestMainWindow::TestMainWindow() : Window {1, 1, __FILE__, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN}
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
			[](GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/,
					const GLchar * message, const void *) {
				const auto msg = uasprintf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
						(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
				switch (type) {
					case GL_DEBUG_TYPE_ERROR:
					case GL_DEBUG_TYPE_PORTABILITY:
					case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
						BOOST_TEST_ERROR(msg.get());
					case GL_DEBUG_TYPE_PERFORMANCE:
						BOOST_TEST_WARN(msg.get());
					default:
						BOOST_TEST_MESSAGE(msg.get());
				}
			},
			nullptr);
}
