#include "svgIcon.h"
#include <resource.h>

SvgIcon::SvgIcon(ImageDimensions dim, const std::filesystem::path & path)
{
	const auto svgDoc = lunasvg::Document::loadFromFile(Resource::mapPath(path).native());
	if (!svgDoc) {
		throw std::runtime_error("Failed to load SVG from " + path.string());
	}

	auto bitmap = svgDoc->renderToBitmap(dim.x, dim.y);
	if (bitmap.isNull()) {
		throw std::runtime_error("Failed to render SVG " + path.string());
	}
	bitmap.convertToRGBA();

	texture.bind();

	texture.parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	texture.parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	texture.parameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture.parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim.x, dim.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.data());
}

ImTextureID
SvgIcon::operator*() const
{
	static_assert(sizeof(glTexture) <= sizeof(ImTextureID));
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr) This is how ImGui works
	return reinterpret_cast<ImTextureID>(*texture);
}
