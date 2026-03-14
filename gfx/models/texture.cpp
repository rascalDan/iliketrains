#include "texture.h"
#include "config/types.h"
#include <fcntl.h>
#include <filesystem.h>
#include <gfx/image.h>
#include <gl_traits.h>
#include <glad/gl.h>
#include <glm/geometric.hpp>
#include <resource.h>
#include <stb/stb_image.h>
#include <sys/mman.h>

using std::ceil;

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
	Texture {static_cast<GLsizei>(tex.width), static_cast<GLsizei>(tex.height), GL_RGBA, GL_UNSIGNED_BYTE,
			tex.data.data(), to}
{
}

Texture::Texture(GLsizei width, GLsizei height, TextureOptions to)
{
	const auto levels = static_cast<GLsizei>(ceil(std::log2(std::max(width, height))));
	m_texture.storage(levels, GL_RGBA8, {width, height});
	m_texture.parameter(GL_TEXTURE_WRAP_S, TextureOptions::glMapMode(to.wrapU));
	m_texture.parameter(GL_TEXTURE_WRAP_T, TextureOptions::glMapMode(to.wrapV));
	m_texture.parameter(GL_TEXTURE_MIN_FILTER, to.minFilter);
	m_texture.parameter(GL_TEXTURE_MAG_FILTER, to.magFilter);
}

Texture::Texture(GLsizei width, GLsizei height, GLenum pixelFormat, GLenum PixelType, const void * pixels,
		TextureOptions to) : Texture {width, height, to}
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	m_texture.image({width, height}, pixelFormat, PixelType, pixels);
	auto isMimmap = [](auto value) {
		auto eqAnyOf = [value](auto... test) {
			return (... || (value == test));
		};
		return eqAnyOf(
				GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	};
	const auto levels = static_cast<GLsizei>(ceil(std::log2(std::max(width, height))));
	if (levels > 1 && (isMimmap(to.minFilter) || isMimmap(to.magFilter))) {
		m_texture.generateMipmap();
	}
}

void
Texture::bind(GLenum unit) const
{
	m_texture.bind(unit);
}

TextureAtlas::TextureAtlas(GLsizei width, GLsizei height, GLuint count) : Texture(width, height)
{
	m_atlas.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_atlas.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_atlas.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_atlas.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_atlas.storage(1, GL_RGBA16UI, {2, count});
}

void
TextureAtlas::bind(GLenum unit) const
{
	Texture::bind(unit);
	m_atlas.bind(unit + 1);
}

GLuint
TextureAtlas::add(TextureAbsCoord position, TextureAbsCoord size, void * data, TextureOptions to)
{
	m_texture.subImage(position, size, GL_RGBA, GL_UNSIGNED_BYTE, data);

	struct Material {
		glm::vec<2, uint16_t> position, size;
		TextureOptions::MapMode wrapU;
		TextureOptions::MapMode wrapV;
	} material {position, size, to.wrapU, to.wrapV};

	static_assert(sizeof(Material) <= 32);
	m_atlas.subImage({0, used}, {2, 1}, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, &material);
	return ++used;
}

void
TextureAtlas::complete()
{
	m_texture.generateMipmap();
}
