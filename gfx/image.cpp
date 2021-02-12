#include "image.h"
#include <cstddef>
#include <stb_image.h>
#include <stdexcept>

Image::Image(const char * fileName, int flags) : width {}, height {}, numComponents {}
{
	stbi_set_flip_vertically_on_load(1);
	unsigned char * bytes = stbi_load(fileName, &width, &height, &numComponents, flags);

	if (!bytes) {
		throw std::runtime_error {std::string {"Unable to load image: "} + fileName};
	}

	data = {bytes, static_cast<size_t>(width * height * numComponents)};
}

Image::~Image()
{
	stbi_image_free(data.data());
}
