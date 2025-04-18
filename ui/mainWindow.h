#pragma once

#include "window.h"
#include <cstddef>

class MainWindow : public Window {
public:
	MainWindow(ScreenAbsCoord size, const char * title, Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	~MainWindow() override;

	NO_MOVE(MainWindow);
	NO_COPY(MainWindow);
};
