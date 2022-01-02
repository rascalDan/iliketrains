#pragma once

#include <GL/glew.h>
#include <array>
#include <cache.h>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glArrays.h>
#include <glm/glm.hpp>
#include <map>
#include <string_view>
#include <vector>

class Font {
public:
	static Cache<Font, std::filesystem::path, unsigned int> cachedFontRenderings;

	Font(std::filesystem::path path, unsigned int height);

	using Quad = std::array<glm::vec4, 4>;
	using Quads = std::vector<Quad>;
	using TextureQuads = std::map<GLuint /*textureId*/, Quads>;
	TextureQuads render(const std::string_view text) const;

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
	void generateChars(const std::string_view text) const;
	const CharData getChar(char) const;
	std::size_t getTextureWithSpace(unsigned int adv) const;

	std::filesystem::path path;
	glm::uvec3 size;
	mutable std::map<uint32_t, CharData> charsData;
	mutable std::vector<FontTexture> fontTextures;
};
