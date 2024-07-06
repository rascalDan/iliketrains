#pragma once

#include "font.h"
#include "glContainer.h"
#include "uiComponent.h"
#include <glArrays.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string_view>

class UIShader;
union SDL_Event;

class Text : public UIComponent {
public:
	Text(std::string_view s, const Font &, Position, glm::vec3 colour);

	void render(const UIShader &, const Position & parentPos) const override;
	bool handleInput(const SDL_Event &, const Position & parentPos) override;

	Text & operator=(const std::string_view s);

private:
	struct TextData {
		GLuint textureId;
		std::vector<unsigned short> range;
	};

	std::vector<TextData> models;
	glContainer<Font::Quad> quads;
	glVertexArray vao;
	glm::vec3 colour;
	const Font & font;
};
