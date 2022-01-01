#ifndef TEXTURE_H
#define TEXTURE_H

#include <filesystem>
#include <glArrays.h>

template<typename Obj> class Cache;

class Texture {
public:
	explicit Texture(const std::filesystem::path & fileName);

	static Cache<Texture> cachedTexture;

	void Bind() const;

private:
	glTexture m_texture;
};

#endif
