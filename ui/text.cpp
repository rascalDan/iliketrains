#include "text.h"
#include "font.h"
#include "gfx/gl/uiShader.h"
#include "gfx/gl/vertexArrayObject.h"
#include "uiComponent.h"
#include <array>
#include <collections.h>
#include <glArrays.h>
#include <glm/gtc/type_ptr.hpp>
#include <maths.h>
#include <numeric>
#include <utility>

Text::Text(std::string_view s, const Font & font, Position pos, glm::vec3 c) :
	UIComponent {pos}, colour {c}, font {font}
{
	VertexArrayObject {vao}.addAttribs<Font::Quad::value_type>(quads.bufferName(), 0);
	operator=(s);
}

Text &
Text::operator=(const std::string_view s)
{
	auto tquads = font.render(s);
	models.resize(tquads.size());
	const auto glyphCount = std::accumulate(tquads.begin(), tquads.end(), size_t {}, [](auto && init, const auto & q) {
		return init += q.second.size();
	});
	quads.resize(glyphCount);
	GLint current = 0;
	auto model = models.begin();
	auto quad = quads.begin();
	for (const auto & [texture, fquads] : tquads) {
		model->first = texture;
		model->second = {fquads.size() * 4, current * 4};
		current += static_cast<GLint>(fquads.size());
		model++;
		quad = std::transform(fquads.begin(), fquads.end(), quad, [this](const Font::Quad & q) {
			return q * [this](const glm::vec4 & corner) {
				return corner + glm::vec4 {this->position.origin, 0, 0};
			};
		});
	}
	quads.unmap();
	return *this;
}

void
Text::render(const UIShader & shader, const Position &) const
{
	shader.text.use(colour);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao);
	for (const auto & m : models) {
		glBindTexture(GL_TEXTURE_2D, m.first);
		glDrawArrays(GL_QUADS, m.second.second, m.second.first);
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool
Text::handleInput(const SDL_Event &, const Position &)
{
	return false;
}
