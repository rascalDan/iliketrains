#include "icon.h"
#include "gl_traits.h"
#include <gfx/image.h>
#include <glad/gl.h>
#include <resource.h>
#include <stb/stb_image.h>

Icon::Icon(const std::filesystem::path & fileName) : Icon {Image {Resource::mapPath(fileName).c_str(), STBI_rgb_alpha}}
{
}

Icon::Icon(const Image & tex) : size {tex.width, tex.height}
{
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(tex.width), static_cast<GLsizei>(tex.height), 0,
			GL_RGBA, GL_UNSIGNED_BYTE, tex.data.data());
}

void
Icon::Bind() const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}
