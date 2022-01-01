#ifndef FONT_H
#define FONT_H

#include <GL/glew.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <string_view>
#include <vector>

class Font {
public:
	Font(const char * const path, unsigned int height);

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
		GLuint texture;
		unsigned int used;
	};

	static glm::uvec3 getTextureSize(unsigned int height);

protected:
	void generateChars(const std::string_view text) const;
	const CharData getChar(char) const;
	std::size_t getTextureWithSpace(unsigned int adv) const;

	std::string path;
	glm::uvec3 size;
	mutable std::map<uint32_t, CharData> charsData;
	mutable std::vector<FontTexture> fontTextures;
};

#endif
