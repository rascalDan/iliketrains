#pragma once

#include "chronology.h"
#include "collection.h"
#include "gfx/gl/uiShader.h"
#include "special_members.h"
#include "stdTypeDefs.h"
#include "uiComponent.h" // IWYU pragma: keep
#include <functional>

class WindowContent : public StdTypeDefs<WindowContent> {
public:
	using Factory = std::function<Ptr(size_t width, size_t height)>;
	WindowContent(size_t width, size_t height);
	virtual ~WindowContent() = default;
	NO_MOVE(WindowContent);
	NO_COPY(WindowContent);

	virtual void tick(TickDuration);
	virtual void render() const = 0;
	virtual bool handleInput(const SDL_Event & e);

protected:
	::Collection<UIComponent> uiComponents;
	UIShader uiShader;
};