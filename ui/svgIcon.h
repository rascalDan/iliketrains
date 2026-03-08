#pragma once

#include "gfx/gl/glTexture.h"
#include <config/types.h>
#include <filesystem>
#include <imgui.h>
#include <lunasvg.h>

class SvgIcon {
public:
	SvgIcon(ImageDimensions, const std::filesystem::path &);

	ImTextureID operator*() const;

private:
	friend class LoadFromFile; // Test case verifying size/content
	glTexture<GL_TEXTURE_2D> texture;
};
