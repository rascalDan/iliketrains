#include "texture.h"
#include "glArrays.h"
#include "tga.h"
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

GLint
TextureOptions::glMapMode(TextureOptions::MapMode mm)
{
	switch (mm) {
		case MapMode::Repeat:
			return GL_REPEAT;
		case MapMode::Clamp:
			return GL_CLAMP_TO_EDGE;
		case MapMode::Mirror:
			return GL_MIRRORED_REPEAT;
		default:
			throw std::domain_error("Invalid MapMode value");
	}
}

Texture::Texture(const std::filesystem::path & fileName, TextureOptions to) :
	Texture {Image {Resource::mapPath(fileName).c_str(), STBI_rgb_alpha}, to}
{
}

Texture::Texture(const Image & tex, TextureOptions to) :
	Texture {static_cast<GLsizei>(tex.width), static_cast<GLsizei>(tex.height), tex.data.data(), to}
{
}

Texture::Texture(GLsizei width, GLsizei height, TextureOptions to) : Texture {width, height, nullptr, to} { }

Texture::Texture(GLsizei width, GLsizei height, const void * data, TextureOptions to) : type {to.type}
{
	glBindTexture(type, m_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(type, GL_TEXTURE_WRAP_S, TextureOptions::glMapMode(to.wrapU));
	glTexParameteri(type, GL_TEXTURE_WRAP_T, TextureOptions::glMapMode(to.wrapV));

	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, to.minFilter);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, to.magFilter);
	glTexImage2D(type, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void
Texture::bind(GLenum unit) const
{
	glActiveTexture(unit);
	glBindTexture(type, m_texture);
}

glm::ivec2
Texture::getSize(const glTexture & texture)
{
	glm::ivec2 size;
	glGetTextureLevelParameteriv(texture, 0, GL_TEXTURE_WIDTH, &size.x);
	glGetTextureLevelParameteriv(texture, 0, GL_TEXTURE_HEIGHT, &size.y);
	return size;
}

void
Texture::save(
		const glTexture & texture, GLenum format, GLenum type, uint8_t channels, const char * path, uint8_t tgaFormat)
{
	const auto size = getSize(texture);
	const size_t dataSize = (static_cast<size_t>(size.x * size.y * channels));
	const size_t fileSize = dataSize + sizeof(TGAHead);

	filesystem::fh out {path, O_RDWR | O_CREAT, 0660};
	out.truncate(fileSize);
	auto tga = out.mmap(fileSize, 0, PROT_WRITE, MAP_SHARED);
	*tga.get<TGAHead>() = {
			.format = tgaFormat,
			.size = size,
			.pixelDepth = static_cast<uint8_t>(8 * channels),
	};
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTextureImage(texture, 0, format, type, static_cast<GLsizei>(dataSize), tga.get<TGAHead>() + 1);
	tga.msync(MS_ASYNC);
}

void
Texture::save(const char * path) const
{
	save(m_texture, GL_BGR, GL_UNSIGNED_BYTE, 3, path, 2);
}

void
Texture::save(const glTexture & texture, const char * path)
{
	save(texture, GL_BGR, GL_UNSIGNED_BYTE, 3, path, 2);
}

void
Texture::saveDepth(const glTexture & texture, const char * path)
{
	save(texture, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 1, path, 3);
}

void
Texture::saveNormal(const glTexture & texture, const char * path)
{
	save(texture, GL_BGR, GL_BYTE, 3, path, 2);
}

TextureAtlas::TextureAtlas(GLsizei width, GLsizei height, GLuint count) : Texture(width, height, nullptr, {})
{
	glBindTexture(GL_TEXTURE_RECTANGLE, m_atlas);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16UI, 2, static_cast<GLsizei>(count), 0, GL_RGBA_INTEGER,
			GL_UNSIGNED_BYTE, nullptr);
}

void
TextureAtlas::bind(GLenum unit) const
{
	Texture::bind(unit);
	glActiveTexture(unit + 1);
	glBindTexture(GL_TEXTURE_RECTANGLE, m_atlas);
}

GLuint
TextureAtlas::add(glm::ivec2 position, glm::ivec2 size, void * data, TextureOptions)
{
	glTextureSubImage2D(m_texture, 0, position.x, position.y, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, data);
	struct Material {
		glm::vec<2, uint16_t> position, size, unused1 {}, unused2 {};
	} material {position, size};
	glTextureSubImage2D(m_atlas, 0, 0, static_cast<GLsizei>(used), 2, 1, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, &material);
	return ++used;
}
