#include "applicationBase.h"
#include "imgui_wrap.h"
#include <SDL2/SDL.h>
#include <stdexcept>

ApplicationBase::ApplicationBase()
{
	initSDL();
	initImGUI();
}

void
ApplicationBase::initSDL() const
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		throw std::runtime_error(SDL_GetError());
	}

	const auto setGlAttribute = [](auto attr, auto value) {
		if (SDL_GL_SetAttribute(attr, value) < 0) {
			throw std::runtime_error(SDL_GetError());
		}
	};
	setGlAttribute(SDL_GL_RED_SIZE, 8);
	setGlAttribute(SDL_GL_GREEN_SIZE, 8);
	setGlAttribute(SDL_GL_BLUE_SIZE, 8);
	setGlAttribute(SDL_GL_ALPHA_SIZE, 8);
	setGlAttribute(SDL_GL_BUFFER_SIZE, 32);
	setGlAttribute(SDL_GL_DEPTH_SIZE, 16);
	setGlAttribute(SDL_GL_DOUBLEBUFFER, 1);

	setGlAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	setGlAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	setGlAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	setGlAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
}

void
ApplicationBase::initImGUI() const
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable viewports
	io.IniFilename = nullptr; // Disable saving settings automagically
}

ApplicationBase::~ApplicationBase()
{
	SDL_Quit();
	ImGui::DestroyContext();
}
