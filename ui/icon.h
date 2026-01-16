#pragma once

#include <filesystem>
#include <glArrays.h>
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
	glTexture m_texture;
};
