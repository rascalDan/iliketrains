#pragma once

#include "gameMainSelector.h"
#include "worldOverlay.h"
#include <game/gamestate.h>
#include <game/network/network.h>
#include <gfx/models/texture.h>

template<typename> class Ray;

class EditNetwork : public GameMainSelector::Component, public WorldOverlay {
public:
	explicit EditNetwork(Network *);

	bool click(const SDL_MouseButtonEvent &, const Ray<GlobalPosition3D> &) override;
	bool move(const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> &) override;
	bool handleInput(const SDL_Event &) override;
	void render(const SceneShader &, const Frustum &) const override;
	void render(bool & open) override;

private:
	[[nodiscard]] std::optional<CreationDefinitionEnd> resolveRay(const Ray<GlobalPosition3D> &) const;

	Network * network;
	bool continuousMode {false};
	SnapPoints snapPoints;
	std::optional<CreationDefinitionEnd> currentStart;
	Link::Collection candidates;
};

template<typename T> class EditNetworkOf : public EditNetwork {
public:
	template<typename... P>
	explicit EditNetworkOf(P &&... params) : EditNetwork(gameState->world.findOrCreate<T>(), std::forward<P>(params)...)
	{
	}
};
