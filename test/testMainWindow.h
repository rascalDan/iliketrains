#pragma once

#include "ui/applicationBase.h"
#include "ui/mainWindow.h"

class TestMainWindow : public MainWindow {
	// This exists only to hold an OpenGL context open for the duration of the tests,
	// in the same way a real main window would always exist.
public:
	TestMainWindow();
};

class TestMainWindowAppBase : public ApplicationBase, public TestMainWindow { };
