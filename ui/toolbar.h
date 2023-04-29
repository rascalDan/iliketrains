#pragma once

#include "collection.h"
#include "iconButton.h"
#include "uiComponent.h"
#include <initializer_list>
#include <string>
#include <utility>

class UIShader;
union SDL_Event;

class Toolbar : public UIComponent {
public:
	using InitInfo = std::pair<std::string, UIEvent>;
	explicit Toolbar(const std::initializer_list<InitInfo> & initInfo);

	void render(const UIShader & uiShader, const Position & parentPos) const override;

	bool handleInput(const SDL_Event & e, const Position & parentPos) override;

	Collection<IconButton, false> icons;
};
