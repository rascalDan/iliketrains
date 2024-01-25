#include "texture.h"
#include "config/types.h"
#include "glArrays.h"
#include "tga.h"
#include <fcntl.h>
#include <filesystem.h>
#include <gfx/image.h>
#include <gl_traits.h>
#include <glad/gl.h>
#include <glm/geometric.hpp>
#include <resource.h>
#include <stb/stb_image.h>
#include <sys/mman.h>

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

	glTexParameter(type, GL_TEXTURE_WRAP_S, TextureOptions::glMapMode(to.wrapU));
	glTexParameter(type, GL_TEXTURE_WRAP_T, TextureOptions::glMapMode(to.wrapV));

	glTexParameter(type, GL_TEXTURE_MIN_FILTER, to.minFilter);
	glTexParameter(type, GL_TEXTURE_MAG_FILTER, to.magFilter);
	glTexImage2D(type, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void
Texture::bind(GLenum unit) const
{
	glActiveTexture(unit);
	glBindTexture(type, m_texture);
}

TextureAbsCoord
Texture::getSize(const glTexture & texture)
{
	TextureAbsCoord size;
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
Texture::savePosition(const glTexture & texture, const char * path)
{
	save(texture, GL_BGR_INTEGER, GL_UNSIGNED_BYTE, 3, path, 2);
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

	glTexParameter(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameter(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameter(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameter(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
TextureAtlas::add(TextureAbsCoord position, TextureAbsCoord size, void * data, TextureOptions to)
{
	glTextureSubImage2D(m_texture, 0, position.x, position.y, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, data);

	struct Material {
		glm::vec<2, uint16_t> position, size;
		TextureOptions::MapMode wrapU;
		TextureOptions::MapMode wrapV;
	} material {position, size, to.wrapU, to.wrapV};

	static_assert(sizeof(Material) <= 32);
	glTextureSubImage2D(m_atlas, 0, 0, static_cast<GLsizei>(used), 2, 1, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, &material);
	return ++used;
}
