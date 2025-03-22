#pragma once

#include "applicationBase.h"
#include "collection.h"
#include "window.h"

class MainApplication : public ApplicationBase {
public:
	using Windows = SharedCollection<Window>;
	void mainLoop();

protected:
	Windows windows;

private:
	void processInputs();
	bool isRunning {true};
};
