#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <special_members.hpp>
#include <string>

template<typename Obj> class Cache;

class Texture {
public:
	explicit Texture(const std::string & fileName);

	virtual ~Texture();

	NO_COPY(Texture);
	NO_MOVE(Texture);

	static Cache<Texture> cachedTexture;

	void Bind() const;

private:
	GLuint m_texture;
};

#endif
