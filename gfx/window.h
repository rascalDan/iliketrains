#ifndef DISPLAY_INCLUDED_H
#define DISPLAY_INCLUDED_H

#include "ptr.hpp"
#include <SDL2/SDL.h>
#include <special_members.hpp>
#include <string>
#include <type_traits>

class Window {
public:
	Window(int width, int height, const std::string & title);
	~Window() = default;

	NO_COPY(Window);
	NO_MOVE(Window);

	void Clear(float r, float g, float b, float a) const;
	void SwapBuffers() const;

private:
	wrapped_ptr<SDL_Window> m_window;
	wrapped_ptr<std::remove_pointer_t<SDL_GLContext>> m_glContext;
};

#endif
