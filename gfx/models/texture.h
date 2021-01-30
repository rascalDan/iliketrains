#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <filesystem>
#include <special_members.hpp>

template<typename Obj> class Cache;

class Texture {
public:
	explicit Texture(const std::filesystem::path & fileName);

	virtual ~Texture();

	NO_COPY(Texture);
	NO_MOVE(Texture);

	static Cache<Texture> cachedTexture;

	void Bind() const;

private:
	GLuint m_texture;
};

#endif
