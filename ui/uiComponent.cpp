#include "uiComponent.h"
#include <SDL2/SDL.h>

UIComponent::UIComponent(Position position) : position {position} { }

UIComponent::Position
UIComponent::Position::operator+(const Position & parentPos) const
{
	return *this + parentPos.origin;
}

UIComponent::Position
UIComponent::Position::operator+(const glm::vec2 & parentPos) const
{
	return {origin + parentPos, size};
}

bool
UIComponent::Position::operator&(const glm::vec2 & pos) const
{
	return (pos.x >= origin.x && pos.y >= origin.y && pos.x < origin.x + size.x && pos.y < origin.y + size.y);
}
bool
UIComponent::Position::operator&(const SDL_MouseButtonEvent & pos) const
{
	switch (pos.type) {
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			return *this & glm::vec2 {pos.x, pos.y};
	}
	return false;
}
