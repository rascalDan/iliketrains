#ifndef ICON_H
#define ICON_H

#include <filesystem>
#include <glArrays.h>
#include <glm/glm.hpp>

class Image;

class Icon {
public:
	explicit Icon(const std::filesystem::path & fileName);
	explicit Icon(const Image & image);

	void Bind() const;
	const glm::vec2 size;

private:
	glTexture m_texture;
};

#endif
