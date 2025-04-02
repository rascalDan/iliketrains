#pragma once

#include "glArrays.h"
#include "imgui_wrap.h"
#include <config/types.h>
#include <filesystem>
#include <lunasvg.h>

class SvgIcon {
public:
	SvgIcon(ImageDimensions, const std::filesystem::path &);

	ImTextureID operator*() const;

private:
	friend class LoadFromFile; // Test case verifying size/content
	glTexture texture;
};
