#pragma once

#include "window.h"
#include <cstddef>

class MainWindow : public Window {
public:
	MainWindow(size_t w, size_t h);
	~MainWindow() override;

	NO_MOVE(MainWindow);
	NO_COPY(MainWindow);

protected:
	MainWindow(size_t width, size_t height, const std::string & title, Uint32 flags);
};
