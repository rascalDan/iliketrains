#pragma once

#include "gfx/gl/glTexture.h"
#include <filesystem>
#include <glm/glm.hpp>
#include <imgui.h>

class Image;

class Icon {
public:
	explicit Icon(const std::filesystem::path & fileName);
	explicit Icon(const Image & image);

	const glm::vec2 size;
	ImTextureID operator*() const;

private:
	glTexture<GL_TEXTURE_2D> m_texture;
};
