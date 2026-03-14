#pragma once

#include "config/types.h"
#include "gfx/gl/glTexture.h"
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
	static GLint glMapMode(MapMode);
};

class Texture : public StdTypeDefs<Texture> {
public:
	virtual ~Texture() = default;
	DEFAULT_MOVE_NO_COPY(Texture);

	explicit Texture(const std::filesystem::path & fileName, TextureOptions = {});
	explicit Texture(const Image & image, TextureOptions = {});
	explicit Texture(GLsizei width, GLsizei height, TextureOptions = {});
	explicit Texture(GLsizei width, GLsizei height, GLenum pixelFormat, GLenum PixelType, const void * pixels,
			TextureOptions = {});

	virtual void bind(GLuint unit) const;

protected:
	glTexture<GL_TEXTURE_2D> m_texture;
};

class TextureAtlas : public Texture {
public:
	TextureAtlas(GLsizei width, GLsizei height, GLuint count);

	void bind(GLuint unit) const override;
	GLuint add(TextureAbsCoord position, TextureAbsCoord size, void * data, TextureOptions = {});
	void complete();

private:
	glTexture<GL_TEXTURE_RECTANGLE> m_atlas;
	GLuint used {};
};
