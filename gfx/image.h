#pragma once

#include <span> // IWYU pragma: export
#include <special_members.hpp>
#include <string>

class Image {
public:
	Image(const char * fileName, int flags);
	Image(const std::string & fileName, int flags) : Image(fileName.c_str(), flags) { }
	~Image();

	NO_COPY(Image);
	NO_MOVE(Image);

	unsigned int width, height, numComponents;
	std::span<unsigned char> data;
};
