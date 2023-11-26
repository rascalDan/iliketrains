#pragma once

#include "SDL_events.h"
#include "config/types.h"
#include "special_members.h"
#include "uiComponent.h"
#include "worldOverlay.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

class SceneShader;
class Ray;
class UIShader;
class Camera;

class GameMainSelector : public UIComponent, public WorldOverlay {
public:
	class Component {
	public:
		virtual ~Component() = default;

		virtual bool click(const SDL_MouseButtonEvent &, const Ray &);
		virtual bool move(const SDL_MouseMotionEvent &, const Ray &);
		virtual bool handleInput(const SDL_Event &, const Position & pos);
		virtual void render(const UIShader & shader, const Position & pos) const;
		virtual void render(const SceneShader &) const;
	};

	GameMainSelector(const Camera * c, ScreenAbsCoord size);

	void render(const UIShader & shader, const Position & pos) const override;
	void render(const SceneShader & shader) const override;

	bool handleInput(const SDL_Event & e, const Position &) override;

	void defaultClick(const Ray & ray);

	std::unique_ptr<Component> target;

private:
	const Camera * camera;
	std::string clicked;
};
