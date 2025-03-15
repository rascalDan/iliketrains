#pragma once

#include "imgui_wrap.h"
#include <filesystem>
#include <glArrays.h>
#include <glm/glm.hpp>

class Image;

class Icon {
public:
	explicit Icon(const std::filesystem::path & fileName);
	explicit Icon(const Image & image);

	void Bind() const;
	const glm::vec2 size;
	ImTextureID operator*() const;

private:
	glTexture m_texture;
};
