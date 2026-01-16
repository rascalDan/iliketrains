#include <imgui.h>

namespace IltGui {
	// NOLINTBEGIN(readability-identifier-naming)
	bool BeginToolbar(const char * name, ImGuiViewport * viewport, ImGuiDir dir, float axisSize,
			ImGuiWindowFlags windowFlags = 0);
	bool BeginToolbar(const char * name, ImGuiDir dir, float axisSize, ImGuiWindowFlags windowFlags = 0);
	void EndToolbar();
	// NOLINTEND(readability-identifier-naming)
}
