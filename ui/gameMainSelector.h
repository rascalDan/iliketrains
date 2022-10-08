#pragma once

#include "SDL_events.h"
#include "special_members.hpp"
#include "uiComponent.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
class Ray;
class UIShader;
class Camera;

class GameMainSelector : public UIComponent {
public:
	class Component {
	public:
		virtual ~Component() = default;

		virtual bool click(const SDL_MouseButtonEvent &, const Ray &);
		virtual bool move(const SDL_MouseMotionEvent &, const Ray &);
		virtual bool handleInput(const SDL_Event &, const Position & pos);
		virtual void render(const UIShader & shader, const Position & pos) const;
	};

	GameMainSelector(const Camera * c, glm::vec2 size);

	void render(const UIShader & shader, const Position & pos) const override;

	bool handleInput(const SDL_Event & e, const Position &) override;

	void defaultClick(const Ray & ray);

	std::unique_ptr<Component> target;

private:
	const Camera * camera;
	std::string clicked;
};
