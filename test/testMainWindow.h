#pragma once

#include "ui/window.h"

class TestMainWindow : public Window {
	// This exists only to hold an OpenGL context open for the duration of the tests,
	// in the same way a real main window would always exist.
public:
	TestMainWindow();

	void
	tick(TickDuration) override
	{
	}
};
