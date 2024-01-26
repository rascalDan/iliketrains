#pragma once

#include "font.h"
#include "uiComponent.h"
#include <glArrays.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string_view>
#include <vector>

class UIShader;
union SDL_Event;

class Text : public UIComponent {
public:
	Text(std::string_view s, const Font &, Position, glm::vec3 colour);

	void render(const UIShader &, const Position & parentPos) const override;
	bool handleInput(const SDL_Event &, const Position & parentPos) override;

private:
	struct Model {
		Model(GLuint, GLsizei);
		GLuint texture;
		GLsizei count;
		glVertexArray vao;
		glBuffer vbo;
	};

	std::vector<Model> models;
	glm::vec3 colour;
};
