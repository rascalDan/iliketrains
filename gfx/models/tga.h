#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

template<glm::length_t Channels> struct TGAHead {
	using XY = glm::vec<2, uint16_t>;
	using PixelType = glm::vec<Channels, uint8_t>;

	uint8_t idLength {}, colorMapType {}, format {};
	uint16_t __attribute__((packed)) colorMapFirst {}, colorMapLength {};
	uint8_t colorMapEntrySize {};
	XY origin {}, size {};
	uint8_t pixelDepth {8 * Channels};
	uint8_t descriptor {};
	PixelType data[1] {};
};
