#include <imgui.h>
#include <imgui_internal.h>
#include <memory>
#include <ranges>

namespace IltGui {
	// NOLINTBEGIN(readability-identifier-naming)
	bool BeginToolbar(const char * name, ImGuiViewport * viewport, ImGuiDir dir, float axisSize,
			ImGuiWindowFlags windowFlags = 0);
	bool BeginToolbar(const char * name, ImGuiDir dir, float axisSize, ImGuiWindowFlags windowFlags = 0);
	void EndToolbar();

	void
	Text(std::ranges::contiguous_range auto text, ImGuiTextFlags flags = ImGuiTextFlags_None)
		requires requires {
			{ std::to_address(text.begin()) } -> std::convertible_to<const char *>;
			{ std::to_address(text.end()) } -> std::convertible_to<const char *>;
		}
	{
		ImGui::TextEx(std::to_address(text.begin()), std::to_address(text.end()), flags);
	}

	// NOLINTEND(readability-identifier-naming)
}
