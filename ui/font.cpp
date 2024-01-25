#include "font.h"
#include <algorithm>
#include <cache.h>
#include <cctype>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "gl_traits.h"
#include <glRef.h>
#include <maths.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <unicode.h>
#include <util.h>
#include <utility>

// IWYU pragma: no_forward_declare FT_LibraryRec_

std::string
FT_Error_StringSafe(FT_Error err)
{
	if (const auto errstr = FT_Error_String(err)) {
		return {errstr};
	}
	return std::to_string(err);
}

template<auto Func, typename... Args>
void
FT_Check(Args &&... args)
{
	if (const auto err = Func(std::forward<Args>(args)...)) {
		throw std::runtime_error {std::string {"FreeType error: "} + FT_Error_StringSafe(err)};
	}
}

const std::string BASIC_CHARS = []() {
	std::string chars;
	for (char c {}; c >= 0; c++) {
		if (isgraph(c)) {
			chars += c;
		}
	}
	return chars + "£€²³";
}();

using FT = glRef<FT_Library,
		[]() {
			FT_Library ft {};
			FT_Check<FT_Init_FreeType>(&ft);
			return ft;
		},
		FT_Done_FreeType>;

using Face = glRef<FT_Face,
		[](FT_Library ft, const char * const name) {
			FT_Face face {};
			FT_Check<FT_New_Face>(ft, name, 0, &face);
			return face;
		},
		FT_Done_Face>;

Cache<Font, std::filesystem::path, unsigned int> Font::cachedFontRenderings;

Font::Font(std::filesystem::path p, unsigned s) : path {std::move(p)}, size {getTextureSize(s)}
{
	generateChars(BASIC_CHARS);
}

void
Font::generateChars(const utf8_string_view chars) const
{
	std::optional<FT> ft;
	std::optional<Face> face;

	for (auto codepoint : chars) {
		if (charsData.find(codepoint) == charsData.end()) {
			if (!ft) {
				ft.emplace();
			}
			if (!face) {
				face.emplace(*ft, path.c_str());
				FT_Set_Pixel_Sizes(*face, 0, size.z);
			}
			const FT_UInt glyph_index = FT_Get_Char_Index(*face, codepoint);
			if (FT_Load_Glyph(*face, glyph_index, FT_LOAD_RENDER)) {
				charsData.emplace(codepoint, CharData {});
				continue;
			}

			const auto & glyph = (*face)->glyph;
			const auto textureIdx = getTextureWithSpace(glyph->bitmap.width);
			auto & texture = fontTextures[textureIdx];

			glTexSubImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(texture.used), 0,
					static_cast<GLsizei>(glyph->bitmap.width), static_cast<GLsizei>(glyph->bitmap.rows), GL_RED,
					GL_UNSIGNED_BYTE, glyph->bitmap.buffer);

			const auto & cd = charsData
									  .emplace(codepoint,
											  CharData {textureIdx, {glyph->bitmap.width, glyph->bitmap.rows},
													  {texture.used, 0}, {glyph->bitmap_left, glyph->bitmap_top},
													  glyph->advance.x >> 6})
									  .first->second;
			texture.used += cd.size.x;
		}
	}
}

std::size_t
Font::getTextureWithSpace(unsigned int adv) const
{
	if (auto itr = std::find_if(fontTextures.begin(), fontTextures.end(),
				[adv, this](const FontTexture & ft) {
					return (ft.used + adv) < size.x;
				});
			itr != fontTextures.end()) {
		glBindTexture(GL_TEXTURE_2D, itr->texture);
		return static_cast<std::size_t>(itr - fontTextures.begin());
	}

	auto & texture = fontTextures.emplace_back();
	glBindTexture(GL_TEXTURE_2D, texture.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, GL_RED,
			GL_UNSIGNED_BYTE, nullptr);
	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return fontTextures.size() - 1;
}

glm::uvec3
Font::getTextureSize(unsigned int height)
{
	auto pow2 = [](unsigned int target) {
		unsigned int v {8};
		do {
			v <<= 1;
		} while (v << 1 < GL_MAX_TEXTURE_SIZE && v < target);
		return v;
	};

	constexpr const unsigned int WIDTH_PER_HEIGHT {64};
	return {pow2(height * WIDTH_PER_HEIGHT), pow2(height), height};
}

Font::TextureQuads
Font::render(const utf8_string_view chars) const
{
	constexpr static const std::array<std::pair<glm::vec2, glm::vec2>, 4> C {{
			{{0, 0}, {0, 1}},
			{{1, 0}, {1, 1}},
			{{1, 1}, {1, 0}},
			{{0, 1}, {0, 0}},
	}};

	generateChars(chars);

	glm::vec2 pos {};
	TextureQuads out;
	for (auto codepoint : chars) {
		if (std::isspace(static_cast<int>(codepoint))) {
			pos.x += static_cast<float>(size.y) / 4.F;
			continue;
		}
		const auto & ch = charsData.at(codepoint);
		if (!ch.advance) {
			continue;
		}

		const auto charPos = pos + glm::vec2 {ch.bearing.x, ch.bearing.y - static_cast<int>(ch.size.y)};
		const auto chSize = glm::vec2 {ch.size};

		out[fontTextures[ch.textureIdx].texture].emplace_back(
				transform_array(C, [&chSize, &charPos, &ch, this](const auto & c) {
					return (charPos + (chSize * c.first))
							|| ((glm::vec2 {ch.position} + (glm::vec2 {ch.size} * c.second)) / glm::vec2 {this->size});
				}));

		pos.x += static_cast<float>(ch.advance);
	}
	return out;
}
