#include "applicationBase.h"
#include <SDL2/SDL.h>
#include <stdexcept>

ApplicationBase::ApplicationBase()
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

	setGlAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	setGlAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	setGlAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	setGlAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
}

ApplicationBase::~ApplicationBase()
{
	SDL_Quit();
}
