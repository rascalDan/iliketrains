#pragma once

#include "gameMainSelector.h"

class QueryTool : public GameMainSelector::Component {
public:
	QueryTool();

protected:
	using GameMainSelector::Component::render;

	bool click(const SDL_MouseButtonEvent &, const Ray<GlobalPosition3D> &) override;
	void render(bool & open) override;

private:
	std::string clicked;
};
