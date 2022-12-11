#pragma once

#include <cache.h>
#include <filesystem>
#include <glArrays.h>
#include <glm/fwd.hpp>

// IWYU pragma: no_forward_declare Cache
class Image;

class Texture {
public:
	explicit Texture(const std::filesystem::path & fileName);
	explicit Texture(const Image & image);
	explicit Texture(GLsizei width, GLsizei height, const void * data);

	static Cache<Texture, std::filesystem::path> cachedTexture;

	void bind(GLenum unit = GL_TEXTURE0) const;

	static void save(const glTexture &, const glm::ivec2 & size, const char * path);
	static void saveDepth(const glTexture &, const glm::ivec2 & size, const char * path);
	static void saveNormal(const glTexture &, const glm::ivec2 & size, const char * path);

private:
	static void save(const glTexture &, GLenum, GLenum, const glm::ivec2 & size, unsigned short channels,
			const char * path, short tgaFormat);

	glTexture m_texture;
};
