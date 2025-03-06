#pragma once

#include "SDL_events.h"
#include "config/types.h"
#include "font.h"
#include "uiComponent.h"
#include "worldOverlay.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

class SceneShader;
template<typename> class Ray;
class UIShader;
class Camera;

class GameMainSelector : public UIComponent, public WorldOverlay {
public:
	class Component {
	public:
		virtual ~Component() = default;

		virtual bool click(const SDL_MouseButtonEvent &, const Ray<GlobalPosition3D> &);
		virtual bool move(const SDL_MouseMotionEvent &, const Ray<GlobalPosition3D> &);
		virtual bool handleInput(const SDL_Event &, const Position & pos);
		virtual void render(const UIShader & shader, const Position & pos) const;
		virtual void render(const SceneShader &, const Frustum &) const;
	};

	GameMainSelector(const Camera * c, ScreenAbsCoord size);

	void render(const UIShader & shader, const Position & pos) const override;
	void render(const SceneShader & shader, const Frustum &) const override;

	bool handleInput(const SDL_Event & e, const Position &) override;

	void defaultClick(const Ray<GlobalPosition3D> & ray);

	std::unique_ptr<Component> target;

private:
	const Camera * camera;
	const Font font;
	std::string clicked;
};
