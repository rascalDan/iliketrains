#include "texture.h"
#include "glArrays.h"
#include <GL/glew.h>
#include <cache.h>
#include <fcntl.h>
#include <filesystem.h>
#include <gfx/image.h>
#include <glm/geometric.hpp>
#include <resource.h>
#include <stb/stb_image.h>
#include <sys/mman.h>

Cache<Texture, std::filesystem::path> Texture::cachedTexture;

Texture::Texture(const std::filesystem::path & fileName, TextureOptions to) :
	Texture {Image {Resource::mapPath(fileName).c_str(), STBI_rgb_alpha}, to}
{
}

Texture::Texture(const Image & tex, TextureOptions to) :
	Texture {static_cast<GLsizei>(tex.width), static_cast<GLsizei>(tex.height), tex.data.data(), to}
{
}

Texture::Texture(GLsizei width, GLsizei height, TextureOptions to) : Texture {width, height, nullptr, to} { }

Texture::Texture(GLsizei width, GLsizei height, const void * data, TextureOptions to)
{
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to.wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to.wrap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, to.minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to.magFilter);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void
Texture::bind(GLenum unit) const
{
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void
Texture::save(const glTexture & texture, GLenum format, GLenum type, const glm::ivec2 & size, unsigned short channels,
		const char * path, short tgaFormat)
{
	using TGAHead = std::array<short, 9>;

	const size_t dataSize = (static_cast<size_t>(size.x * size.y * channels));
	const size_t fileSize = dataSize + sizeof(TGAHead);

	filesystem::fh out {path, O_RDWR | O_CREAT, 0660};
	out.truncate(fileSize);
	auto tga = out.mmap(fileSize, 0, PROT_WRITE, MAP_SHARED);
	*tga.get<TGAHead>() = {0, tgaFormat, 0, 0, 0, 0, static_cast<short>(size.x), static_cast<short>(size.y),
			static_cast<short>(8 * channels)};
	glGetTextureImage(texture, 0, format, type, static_cast<GLsizei>(dataSize), tga.get<TGAHead>() + 1);
	tga.msync(MS_ASYNC);
}

void
Texture::save(const glTexture & texture, const glm::ivec2 & size, const char * path)
{
	save(texture, GL_BGR, GL_UNSIGNED_BYTE, size, 3, path, 2);
}

void
Texture::saveDepth(const glTexture & texture, const glm::ivec2 & size, const char * path)
{
	save(texture, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, size, 1, path, 3);
}

void
Texture::saveNormal(const glTexture & texture, const glm::ivec2 & size, const char * path)
{
	save(texture, GL_BGR, GL_BYTE, size, 3, path, 2);
}
