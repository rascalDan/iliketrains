#pragma once

#include "foliage.h"
#include "game/worldobject.h"
#include "gfx/renderable.h"
#include "location.hpp"

class Plant : public Renderable, public WorldObject {
	std::shared_ptr<const Foliage> type;
	Location position;

	void render(const SceneShader & shader) const override;
	void shadows(const ShadowMapper & shadowMapper) const override;

	void
	tick(TickDuration) override
	{
	}

public:
	Plant(std::shared_ptr<const Foliage> type, Location position) : type(std::move(type)), position(position) { }
};
