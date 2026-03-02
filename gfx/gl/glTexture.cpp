#include "glTexture.h"

TextureDimensions
Impl::glTexture::getSize() const
{
	TextureDimensions size {};
	glGetTextureLevelParameteriv(name, 0, GL_TEXTURE_WIDTH, &size.x);
	glGetTextureLevelParameteriv(name, 0, GL_TEXTURE_HEIGHT, &size.y);
	glGetTextureLevelParameteriv(name, 0, GL_TEXTURE_DEPTH, &size.z);
	return size;
}

void
Impl::glTexture::bind(GLenum type, GLenum unit) const
{
	glActiveTexture(unit);
	glBindTexture(type, name);
}
