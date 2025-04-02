#pragma once

#include "chronology.h"
#include "collection.h"
#include "special_members.h"
#include "stdTypeDefs.h"
#include "uiComponent.h" // IWYU pragma: keep

class WindowContent : public StdTypeDefs<WindowContent> {
public:
	WindowContent() = default;
	virtual ~WindowContent() = default;
	NO_MOVE(WindowContent);
	NO_COPY(WindowContent);

	virtual void tick(TickDuration);
	virtual void render() const = 0;
	virtual bool handleInput(const SDL_Event & e);

protected:
	UniqueCollection<UIComponent> uiComponents;
};
