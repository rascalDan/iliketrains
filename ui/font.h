#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <glArrays.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <map>
#include <unicode.h>
#include <vector>

class Font {
public:
	Font(std::filesystem::path path, unsigned int height);

	using Quad = std::array<glm::vec4, 4>;
	using Quads = std::vector<Quad>;
	using TextureQuads = std::map<GLuint /*textureId*/, Quads>;
	TextureQuads render(const utf8_string_view text) const;

	struct CharData {
		size_t textureIdx;
		glm::uvec2 size;
		glm::uvec2 position;
		glm::ivec2 bearing;
		long advance;
	};

	struct FontTexture {
		glTexture texture;
		unsigned int used;
	};

	static glm::uvec3 getTextureSize(unsigned int height);

protected:
	void generateChars(const utf8_string_view text) const;
	const CharData getChar(char) const;
	std::size_t getTextureWithSpace(unsigned int adv) const;

	std::filesystem::path path;
	glm::uvec3 size;
	mutable std::map<decltype(get_codepoint(nullptr)), CharData> charsData;
	mutable std::vector<FontTexture> fontTextures;
};
