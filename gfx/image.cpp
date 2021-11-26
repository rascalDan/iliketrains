#include "image.h"
#include <cstddef>
#include <stb/stb_image.h>
#include <stdexcept>

Image::Image(const char * fileName, int flags) : width {}, height {}, numComponents {}
{
	stbi_set_flip_vertically_on_load(1);
	int w, h, nc;
	unsigned char * bytes = stbi_load(fileName, &w, &h, &nc, flags);
	width = static_cast<unsigned int>(w);
	height = static_cast<unsigned int>(h);
	numComponents = static_cast<unsigned int>(nc);

	if (!bytes) {
		throw std::runtime_error {std::string {"Unable to load image: "} + fileName};
	}

	data = {bytes, static_cast<size_t>(width * height * numComponents)};
}

Image::~Image()
{
	stbi_image_free(data.data());
}
