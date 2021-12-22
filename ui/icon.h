#ifndef ICON_H
#define ICON_H

#include <GL/glew.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <special_members.hpp>

class Image;

class Icon {
public:
	explicit Icon(const std::filesystem::path & fileName);
	explicit Icon(const Image & image);

	virtual ~Icon();

	NO_COPY(Icon);
	NO_MOVE(Icon);

	void Bind() const;
	const glm::vec2 size;

private:
	GLuint m_texture;
};

#endif
