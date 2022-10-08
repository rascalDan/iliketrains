#pragma once

#include "gameMainSelector.h"
#include "modeHelper.hpp"
#include "toolbar.h"
#include "worldOverlay.h"
#include <game/gamestate.h>
#include <game/network/network.h>
#include <optional>

class Ray;

class EditNetwork : public GameMainSelector::Component, public WorldOverlay {
public:
	explicit EditNetwork(Network *);
	virtual ~EditNetwork();

	bool click(const SDL_MouseButtonEvent & e, const Ray &) override;
	bool move(const SDL_MouseMotionEvent & e, const Ray &) override;
	bool handleInput(const SDL_Event & e, const UIComponent::Position &) override;
	void render(const Shader &) const override;
	void render(const UIShader & shader, const UIComponent::Position & pos) const override;

	using NetworkClickPos = std::variant<glm::vec3, NodePtr>;

	class Builder {
	public:
		virtual ~Builder() = default;
		virtual void render(const Shader & shader) const = 0;
		virtual std::string hint() const = 0;
		virtual void click(Network *, const GeoData *, const SDL_MouseButtonEvent &, const Ray &) = 0;
	};

private:
	using BuilderPtr = std::unique_ptr<Builder>;

	Network * network;
	BuilderPtr builder;
	Mode<BuilderPtr, ModeSecondClick::NoAction> mode {builder};
	Toolbar builderToolbar;
};

template<typename T> class EditNetworkOf : public EditNetwork {
public:
	template<typename... P>
	EditNetworkOf(P &&... p) : EditNetwork(gameState->world.findOrCreate<T>(), std::forward<P>(p)...)
	{
	}
};
