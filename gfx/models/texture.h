#pragma once

#include <cache.h>
#include <filesystem>
#include <glArrays.h>

// IWYU pragma: no_forward_declare Cache
class Image;

class Texture {
public:
	explicit Texture(const std::filesystem::path & fileName);
	explicit Texture(const Image & image);
	explicit Texture(GLsizei width, GLsizei height, const void * data);

	static Cache<Texture, std::filesystem::path> cachedTexture;

	void Bind() const;

private:
	glTexture m_texture;
};
