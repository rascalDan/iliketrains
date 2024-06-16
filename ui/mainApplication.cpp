#include "mainApplication.h"
#include "game/gamestate.h"
#include "game/worldobject.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "backends/imgui_impl_sdl2.h"
#pragma GCC diagnostic pop

void
MainApplication::mainLoop()
{
	auto t_start = std::chrono::high_resolution_clock::now();
	while (isRunning) {
		processInputs();
		const auto t_end = std::chrono::high_resolution_clock::now();
		const auto t_passed = std::chrono::duration_cast<TickDuration>(t_end - t_start);

		if (gameState) {
			gameState->world.apply(&WorldObject::tick, t_passed);
		}
		windows.apply(&Window::tick, t_passed);
		windows.apply(&Window::refresh);
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		t_start = t_end;
	}
}

void
MainApplication::processInputs()
{
	for (SDL_Event e; SDL_PollEvent(&e);) {
		if (e.type == SDL_QUIT) {
			isRunning = false;
			return;
		}
		ImGui_ImplSDL2_ProcessEvent(&e);
		windows.applyOne(&Window::handleInput, e);
	}
}
