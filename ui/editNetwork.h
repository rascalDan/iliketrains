#pragma once

#include "gameMainSelector.h"
#include "modeHelper.h"
#include "toolbar.h"
#include "worldOverlay.h"
#include <game/gamestate.h>
#include <game/network/network.h>
#include <gfx/models/texture.h>
#include <optional>

class Ray;

class EditNetwork : public GameMainSelector::Component, public WorldOverlay {
public:
	explicit EditNetwork(Network *);

	bool click(const SDL_MouseButtonEvent & e, const Ray &) override;
	bool move(const SDL_MouseMotionEvent & e, const Ray &) override;
	bool handleInput(const SDL_Event & e, const UIComponent::Position &) override;
	void render(const SceneShader &) const override;
	void render(const UIShader & shader, const UIComponent::Position & pos) const override;

	using NetworkClickPos = std::variant<Position3D, Node::Ptr>;

	class Builder {
	public:
		virtual ~Builder() = default;
		virtual void render(const SceneShader & shader) const;
		virtual std::string hint() const = 0;
		virtual void click(Network *, const GeoData *, const SDL_MouseButtonEvent &, const Ray &) = 0;
		virtual void move(Network *, const GeoData *, const SDL_MouseMotionEvent &, const Ray &) = 0;

		using Ptr = std::unique_ptr<Builder>;

	protected:
		Collection<const Link> candidateLinks;
	};

private:
	Network * network;
	Builder::Ptr builder;
	Mode<Builder::Ptr, ModeSecondClick::NoAction> mode {builder};
	Toolbar builderToolbar;
	Texture blue;
};

template<typename T> class EditNetworkOf : public EditNetwork {
public:
	template<typename... P>
	explicit EditNetworkOf(P &&... p) : EditNetwork(gameState->world.findOrCreate<T>(), std::forward<P>(p)...)
	{
	}
};
