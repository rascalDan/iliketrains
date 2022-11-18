#include "texture.h"
#include "glArrays.h"
#include <GL/glew.h>
#include <cache.h>
#include <fcntl.h>
#include <gfx/image.h>
#include <glm/geometric.hpp>
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

Texture::Texture(GLsizei width, GLsizei height, const void * data)
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void
Texture::bind(GLenum unit) const
{
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void
Texture::save(const glTexture & texture, const glm::ivec2 & size, const char * path)
{
	std::vector<unsigned char> buffer(static_cast<size_t>(size.x * size.y * 3));
	glGetTextureImage(texture, 0, GL_BGR, GL_UNSIGNED_BYTE, static_cast<GLsizei>(buffer.size()), buffer.data());

	auto out = open(path, O_WRONLY | O_CREAT, 0660);
	short TGAhead[] = {0, 2, 0, 0, 0, 0, static_cast<short>(size.x), static_cast<short>(size.y), 24};
	std::ignore = write(out, &TGAhead, sizeof(TGAhead));
	std::ignore = write(out, buffer.data(), buffer.size());
	std::ignore = ftruncate(out, static_cast<off_t>(buffer.size() + sizeof(TGAhead)));
	close(out);
}
