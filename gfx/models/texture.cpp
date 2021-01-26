#include "texture.h"
#include "stb_image.h"
#include <cache.h>
#include <stdexcept>

Cache<Texture> Texture::cachedTexture;

Texture::Texture(const std::string & fileName) : m_texture {}
{
	int width, height, numComponents;
	unsigned char * data = stbi_load((fileName).c_str(), &width, &height, &numComponents, STBI_rgb_alpha);

	if (!data) {
		throw std::runtime_error {"Unable to load texture: " + fileName};
	}

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_texture);
}

void
Texture::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
}
