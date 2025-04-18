#pragma once

#include "config/types.h"
#include "glArrays.h"
#include "stdTypeDefs.h"
#include <filesystem>
#include <glm/fwd.hpp>

class Image;

struct TextureOptions {
	enum class MapMode {
		Repeat,
		Clamp,
		Mirror,
		Decal,
	};
	MapMode wrapU {MapMode::Repeat}, wrapV {MapMode::Repeat};
	GLint minFilter {GL_LINEAR}, magFilter {GL_LINEAR};
	GLenum type {GL_TEXTURE_2D};
	static GLint glMapMode(MapMode);
};

class Texture : public StdTypeDefs<Texture> {
public:
	virtual ~Texture() = default;
	DEFAULT_MOVE_NO_COPY(Texture);

	explicit Texture(const std::filesystem::path & fileName, TextureOptions = {});
	explicit Texture(const Image & image, TextureOptions = {});
	explicit Texture(GLsizei width, GLsizei height, TextureOptions = {});
	explicit Texture(GLsizei width, GLsizei height, const void * data, TextureOptions = {});

	virtual void bind(GLenum unit = GL_TEXTURE0) const;

	void save(const char * path) const;
	static void save(const glTexture &, const char * path);
	static void saveDepth(const glTexture &, const char * path);
	static void saveNormal(const glTexture &, const char * path);
	static void savePosition(const glTexture &, const char * path);
	static TextureDimensions getSize(const glTexture &);

protected:
	static void save(const glTexture &, GLenum, GLenum, uint8_t channels, const char * path, uint8_t tgaFormat);

	glTexture m_texture;
	GLenum type;
};

class TextureAtlas : public Texture {
public:
	TextureAtlas(GLsizei width, GLsizei height, GLuint count);

	void bind(GLenum unit = GL_TEXTURE0) const override;
	GLuint add(TextureAbsCoord position, TextureAbsCoord size, void * data, TextureOptions = {});

private:
	glTexture m_atlas;
	GLuint used {};
};
