#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include "icon.h"
#include "uiComponent.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class UIShader;
union SDL_Event;

static const constexpr glm::vec2 ICON_SIZE {32.F, 32.F};
class IconButton : public UIComponent {
public:
	IconButton(const std::string & icon, glm::vec2 position, UIEvent click);

	void render(const UIShader &, const Position & parentPos) const override;

	bool handleInput(const SDL_Event & e, const Position & parentPos) override;

	Icon icon;
	UIEvent click;
	GLuint m_vertexArrayObject, m_vertexArrayBuffer;
};

#endif
