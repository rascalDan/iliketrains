#include "mainApplication.h"
#include "game/gamestate.h"
#include "game/worldobject.h"

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
		windows.applyOne(&Window::handleInput, e);
	}
}
