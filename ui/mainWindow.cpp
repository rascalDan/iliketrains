#include "mainWindow.h"
#include <format>
#include <stdexcept>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#pragma GCC diagnostic pop

MainWindow::MainWindow(ScreenAbsCoord size, const char * title, Uint32 flags) : Window {size, title, flags}
{
	if (const auto version = gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)); version < 30003) {
		throw std::runtime_error {std::format("Insufficient OpenGL version: {}", version)};
	}

	ImGui_ImplSDL2_InitForOpenGL(m_window, glContext.get());
	ImGui_ImplOpenGL3_Init();
}

MainWindow::~MainWindow()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
}
