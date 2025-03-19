#pragma once

#include "gameMainSelector.h"

class QueryTool : public GameMainSelector::Component {
protected:
	using GameMainSelector::Component::render;

	bool click(const SDL_MouseButtonEvent &, const Ray<GlobalPosition3D> &) override;
	void render(const UIShader & shader, const UIComponent::Position & pos) override;

private:
	std::string clicked;
};
