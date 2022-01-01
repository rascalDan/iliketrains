#include "iconButton.h"
#include "glArrays.h"
#include "ui/icon.h"
#include "ui/uiComponent.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <array>
#include <filesystem>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

IconButton::IconButton(const std::string & icon_, glm::vec2 position_, UIEvent click_) :
	UIComponent {{position_, ICON_SIZE}}, icon {icon_}, click {std::move(click_)}
{
	glBindVertexArray(m_vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(glm::vec4) * 4), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);

	glBindVertexArray(0);
}

void
IconButton::render(const UIShader &, const Position & parentPos) const
{
	icon.Bind();
	glBindVertexArray(m_vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexArrayBuffer);
	const auto abs = parentPos.origin + position.origin;
	const auto limit = abs + ICON_SIZE;
	std::array<glm::vec4, 4> vertices {{
			{abs.x, abs.y, 0, 0},
			{limit.x, abs.y, 1, 0},
			{limit.x, limit.y, 1, 1},
			{abs.x, limit.y, 0, 1},
	}};
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), glm::value_ptr(vertices.front()));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

bool
IconButton::handleInput(const SDL_Event & e, const Position & parentPos)
{
	const auto absPos = position + parentPos;
	if (absPos & e.button) {
		if (e.button.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
			click(e);
		}
	}
	return false;
}
