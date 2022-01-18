#pragma once

#include "special_members.hpp"
#include "uiComponent.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>
class Ray;
class UIShader;
union SDL_Event;
class Camera;

class GameMainSelector : public UIComponent {
public:
	class ClickReceiver {
	public:
		ClickReceiver() = default;
		virtual ~ClickReceiver() = default;
		DEFAULT_MOVE_COPY(ClickReceiver);

		virtual void click(const Ray &) = 0;
		virtual void move(const Ray &) = 0;
		virtual bool handleMove() = 0;
	};

	GameMainSelector(const Camera * c, glm::vec2 size);

	void render(const UIShader & shader, const Position & pos) const override;

	bool handleInput(const SDL_Event & e, const Position &) override;

	void defaultClick(const Ray & ray);

	std::unique_ptr<ClickReceiver> target;

private:
	const Camera * camera;
	std::string clicked;
};
