#include "texture.h"
#include "glArrays.h"
#include <GL/glew.h>
#include <cache.h>
#include <gfx/image.h>
#include <resource.h>
#include <stb/stb_image.h>

Cache<Texture, std::filesystem::path> Texture::cachedTexture;

Texture::Texture(const std::filesystem::path & fileName) :
	Texture {Image {Resource::mapPath(fileName).c_str(), STBI_rgb_alpha}}
{
}

Texture::Texture(const Image & tex) :
	Texture {static_cast<GLsizei>(tex.width), static_cast<GLsizei>(tex.height), tex.data.data()}
{
}

Texture::Texture(GLsizei width, GLsizei height, void * data)
{
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void
Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
}
