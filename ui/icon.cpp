#include "icon.h"
#include <gfx/image.h>
#include <glad/gl.h>
#include <resource.h>
#include <stb/stb_image.h>

Icon::Icon(const std::filesystem::path & fileName) : Icon {Image {Resource::mapPath(fileName).c_str(), STBI_rgb_alpha}}
{
}

Icon::Icon(const Image & tex) : size {tex.width, tex.height}
{
	m_texture.bind();

	m_texture.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	m_texture.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	m_texture.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_texture.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(tex.width), static_cast<GLsizei>(tex.height), 0,
			GL_RGBA, GL_UNSIGNED_BYTE, tex.data.data());
}

ImTextureID
Icon::operator*() const
{
	static_assert(sizeof(m_texture) <= sizeof(ImTextureID));
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr) This is how ImGui works
	return reinterpret_cast<ImTextureID>(*m_texture);
}
