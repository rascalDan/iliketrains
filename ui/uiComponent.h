#ifndef UICOMPONENT_H
#define UICOMPONENT_H

#include <functional>
#include <glm/glm.hpp>
#include <special_members.hpp>

class UIShader;
union SDL_Event;
struct SDL_MouseButtonEvent;
using UIEvent = std::function<void(const SDL_Event &)>;

class UIComponent {
public:
	struct Position {
		glm::vec2 origin, size;
		Position operator+(const Position &) const;
		Position operator+(const glm::vec2 &) const;
		bool operator&(const SDL_MouseButtonEvent &) const;
		bool operator&(const glm::vec2 &) const;
	};
	explicit UIComponent(Position);
	virtual ~UIComponent() = default;

	NO_MOVE(UIComponent);
	NO_COPY(UIComponent);

	virtual void render(const UIShader &, const Position & parentPos) const = 0;
	virtual bool handleInput(const SDL_Event &, const Position & parentPos) = 0;

	Position position;
};

#endif
