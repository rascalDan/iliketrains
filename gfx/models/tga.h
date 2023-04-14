#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

struct TGAHead {
	uint8_t idLength {}, colorMapType {}, format {};
	uint16_t __attribute__((packed)) colorMapFirst {}, colorMapLength {};
	uint8_t colorMapEntrySize {};
	glm::vec<2, uint16_t> origin {}, size {};
	uint8_t pixelDepth {};
	uint8_t descriptor {};
};
static_assert(sizeof(TGAHead) == 18);
