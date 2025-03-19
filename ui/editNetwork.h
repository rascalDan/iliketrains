#pragma once

#include "game/geoData.h"
#include "gameMainSelector.h"
#include "toolbar.h"
#include "worldOverlay.h"
#include <game/gamestate.h>
#include <game/network/network.h>
#include <gfx/models/texture.h>

template<typename> class Ray;

class EditNetwork : public GameMainSelector::Component, public WorldOverlay {
public:
	explicit EditNetwork(Network *);

	bool click(const SDL_MouseButtonEvent & e, const Ray<GlobalPosition3D> &) override;
	bool move(const SDL_MouseMotionEvent & e, const Ray<GlobalPosition3D> &) override;
	bool handleInput(const SDL_Event & e, const UIComponent::Position &) override;
	void render(const SceneShader &, const Frustum &) const override;
	void render(const UIShader & shader, const UIComponent::Position & pos) override;

	using NetworkClickPos = std::variant<GlobalPosition3D, Node::Ptr>;

	class Builder {
	public:
		virtual ~Builder() = default;
		virtual void render(const SceneShader & shader, const Frustum &) const;
		virtual std::string hint() const = 0;
		virtual void click(Network *, const GeoData *, const SDL_MouseButtonEvent &, const Ray<GlobalPosition3D> &) = 0;
		virtual void move(Network *, const GeoData *, const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> &) = 0;

		static void setHeightsFor(Network *, const Link::CCollection &, GeoData::SetHeightsOpts = {});

		using Ptr = std::unique_ptr<Builder>;

	protected:
		Collection<const Link> candidateLinks;
	};

private:
	Network * network;
	Builder::Ptr builder;
	Texture blue;
	const Font font;
};

template<typename T> class EditNetworkOf : public EditNetwork {
public:
	template<typename... P>
	explicit EditNetworkOf(P &&... p) : EditNetwork(gameState->world.findOrCreate<T>(), std::forward<P>(p)...)
	{
	}
};
