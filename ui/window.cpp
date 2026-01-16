#include "window.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include <glad/gl.h>
#include <glm/glm.hpp>

Window::Window(ScreenAbsCoord size, const char * title, Uint32 flags) :
	m_window {title, static_cast<int>(SDL_WINDOWPOS_CENTERED), static_cast<int>(SDL_WINDOWPOS_CENTERED), size.x, size.y,
			flags},
	glContext {m_window}
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

void
Window::tick(TickDuration elapsed)
{
	if (content) {
		content->tick(elapsed);
	}
}

bool
Window::handleInput(const SDL_Event & e)
{
	if (SDL_GetWindowID(m_window) == e.window.windowID) {
		if (content) {
			return content->handleInput(e);
		}
	}
	return false;
}

void
Window::refresh() const
{
	SDL_GL_MakeCurrent(m_window, glContext);
	clear(0.0F, 0.0F, 0.0F, 1.0F);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	if (content) {
		content->render();
		// Render UI stuff here
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	swapBuffers();
}
