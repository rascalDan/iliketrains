#include "text.h"
#include "font.h"
#include "gfx/gl/uiShader.h"
#include "uiComponent.h"
#include <array>
#include <cache.h>
#include <filesystem>
#include <glArrays.h>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <memory>
#include <utility>

const std::filesystem::path font {"/usr/share/fonts/hack/Hack-Regular.ttf"};
Text::Text(std::string_view s, Position pos, glm::vec3 c) : UIComponent {pos}, colour {c}
{
	for (const auto & textureQuads :
			Font::cachedFontRenderings.get(font, static_cast<unsigned int>(pos.size.y))->render(s)) {
		auto & rendering
				= models.emplace_back(textureQuads.first, static_cast<GLsizei>(6 * textureQuads.second.size()));
		glBindVertexArray(rendering.vao);

		glBindBuffer(GL_ARRAY_BUFFER, rendering.vbo);
		std::vector<glm::vec4> vertices;
		vertices.reserve(6 * textureQuads.second.size());
		for (const auto & quad : textureQuads.second) {
			for (auto offset = 0U; offset < 3; offset += 2) {
				for (auto vertex = 0U; vertex < 3; vertex += 1) {
					vertices.emplace_back(quad[(vertex + offset) % 4] + glm::vec4 {position.origin, 0, 0});
				}
			}
		};
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(glm::vec4)) * rendering.count,
				glm::value_ptr(vertices.front()), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);

		glBindVertexArray(0);
	}
}

void
Text::render(const UIShader & shader, const Position &) const
{
	shader.useText(colour);
	for (const auto & m : models) {
		glBindTexture(GL_TEXTURE_2D, m.texture);
		glBindVertexArray(m.vao);
		glDrawArrays(GL_TRIANGLES, 0, m.count);
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool
Text::handleInput(const SDL_Event &, const Position &)
{
	return false;
}

Text::Model::Model(GLuint t, GLsizei c) : texture {t}, count {c} { }
