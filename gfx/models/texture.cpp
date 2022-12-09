#include "texture.h"
#include "glArrays.h"
#include <GL/glew.h>
#include <cache.h>
#include <fcntl.h>
#include <gfx/image.h>
#include <glm/geometric.hpp>
#include <resource.h>
#include <stb/stb_image.h>
#include <sys/mman.h>

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
Texture::save(const glTexture & texture, GLenum format, const glm::ivec2 & size, unsigned short channels,
		const char * path, short tgaFormat)
{
	using TGAHead = std::array<short, 9>;

	size_t dataSize = (static_cast<size_t>(size.x * size.y * channels));
	size_t fileSize = dataSize + sizeof(TGAHead);

	auto out = open(path, O_RDWR | O_CREAT, 0660);
	std::ignore = ftruncate(out, static_cast<off_t>(fileSize));
	TGAHead * tga = static_cast<TGAHead *>(mmap(nullptr, fileSize, PROT_WRITE, MAP_SHARED, out, 0));
	close(out);
	if (tga == MAP_FAILED) {
		return;
	}
	*tga = {0, tgaFormat, 0, 0, 0, 0, static_cast<short>(size.x), static_cast<short>(size.y),
			static_cast<short>(8 * channels)};
	glGetTextureImage(texture, 0, format, GL_UNSIGNED_BYTE, static_cast<GLsizei>(dataSize), tga + 1);
	msync(tga, fileSize, MS_ASYNC);
	munmap(tga, fileSize);
}

void
Texture::save(const glTexture & texture, const glm::ivec2 & size, const char * path)
{
	save(texture, GL_BGR, size, 3, path, 2);
}

void
Texture::saveDepth(const glTexture & texture, const glm::ivec2 & size, const char * path)
{
	save(texture, GL_DEPTH_COMPONENT, size, 1, path, 3);
}
