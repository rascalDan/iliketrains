#pragma once

#include <cache.h>
#include <filesystem>
#include <glArrays.h>

// IWYU pragma: no_forward_declare Cache

class Texture {
public:
	explicit Texture(const std::filesystem::path & fileName);

	static Cache<Texture, std::filesystem::path> cachedTexture;

	void Bind() const;

private:
	glTexture m_texture;
};
