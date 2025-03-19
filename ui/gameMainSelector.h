#pragma once

#include "SDL_events.h"
#include "config/types.h"
#include "uiComponent.h"
#include "worldOverlay.h"
#include <glm/glm.hpp>
#include <memory>

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
		virtual bool handleInput(const SDL_Event &);
		virtual void render(const UIShader & shader);
		virtual void render(const SceneShader &, const Frustum &) const;
	};

	GameMainSelector(const Camera * c);

	void render(const UIShader & shader) const override;
	void render(const SceneShader & shader, const Frustum &) const override;

	bool handleInput(const SDL_Event & e) override;

	void defaultClick(const Ray<GlobalPosition3D> & ray);

	std::unique_ptr<Component> target;

private:
	const Camera * camera;
};
