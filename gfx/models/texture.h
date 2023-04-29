#pragma once

#include <cache.h>
#include <filesystem>
#include <glArrays.h>
#include <glm/fwd.hpp>

// IWYU pragma: no_forward_declare Cache
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

class Texture {
public:
	virtual ~Texture() = default;
	DEFAULT_MOVE_NO_COPY(Texture);

	explicit Texture(const std::filesystem::path & fileName, TextureOptions = {});
	explicit Texture(const Image & image, TextureOptions = {});
	explicit Texture(GLsizei width, GLsizei height, TextureOptions = {});
	explicit Texture(GLsizei width, GLsizei height, const void * data, TextureOptions = {});

	static Cache<Texture, std::filesystem::path> cachedTexture;

	virtual void bind(GLenum unit = GL_TEXTURE0) const;

	void save(const char * path) const;
	static void save(const glTexture &, const char * path);
	static void saveDepth(const glTexture &, const char * path);
	static void saveNormal(const glTexture &, const char * path);

protected:
	static void save(const glTexture &, GLenum, GLenum, uint8_t channels, const char * path, uint8_t tgaFormat);
	static glm::ivec2 getSize(const glTexture &);

	glTexture m_texture;
	GLenum type;
};

class TextureAtlas : public Texture {
public:
	TextureAtlas(GLsizei width, GLsizei height, GLuint count);

	void bind(GLenum unit = GL_TEXTURE0) const override;
	GLuint add(glm::ivec2 position, glm::ivec2 size, void * data, TextureOptions = {});

private:
	glTexture m_atlas;
	GLuint used {};
};
