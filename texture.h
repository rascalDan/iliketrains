#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <string>

class Texture {
public:
	explicit Texture(const std::string & fileName);

	Texture(const Texture &) = delete;
	void operator=(const Texture &) = delete;

	virtual ~Texture();

	void Bind() const;

protected:
private:
	GLuint m_texture;
};

#endif
