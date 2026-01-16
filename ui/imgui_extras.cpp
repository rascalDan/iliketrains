#include "imgui_extras.h"
#include <imgui_internal.h>

namespace IltGui {
	bool
	BeginToolbar(const char * name, ImGuiDir dir, float axisSize, ImGuiWindowFlags windowFlags)
	{
		return BeginToolbar(name, ImGui::GetMainViewport(), dir, axisSize, windowFlags);
	}

	bool
	BeginToolbar(
			const char * name, ImGuiViewport * viewport, ImGuiDir dir, float axisSize, ImGuiWindowFlags windowFlags)
	{
		bool isOpen = ImGui::BeginViewportSideBar(name, viewport, dir, axisSize,
				windowFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
		if (isOpen) {
			if (dir == ImGuiDir_Up || dir == ImGuiDir_Down) {
				ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;
			}
		}
		else {
			ImGui::End();
		}
		return isOpen;
	}

	void
	EndToolbar()
	{
		ImGui::End();
	}
}
