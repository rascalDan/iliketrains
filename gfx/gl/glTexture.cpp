#include "glTexture.h"
#include "filesystem.h"
#include "gfx/models/tga.h"
#include <fcntl.h>
#include <ranges>
#include <sys/mman.h>

void
Impl::glTextureBase::bind(const GLuint unit) const
{
	glBindTextureUnit(unit, name);
}

void
Impl::glTextureBase::generateMipmap()
{
	glGenerateTextureMipmap(name);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
glm::vec<Dims, GLsizei>
Impl::glTextureDims<Dims>::getSize() const
{
	static constexpr std::array<GLenum, 3> PARAMS {GL_TEXTURE_WIDTH, GL_TEXTURE_HEIGHT, GL_TEXTURE_DEPTH};
	glm::vec<Dims, GLsizei> size {};
	for (auto [dim, param] : std::views::enumerate(PARAMS) | std::views::take(Dims)) {
		glGetTextureLevelParameteriv(name, 0, param, &size[static_cast<glm::length_t>(dim)]);
	}
	return size;
}

template<>
void
Impl::glTextureDims<1>::storage(const GLsizei levels, const GLenum internalformat, glm::vec<1, GLsizei> dims)
{
	glTextureStorage1D(name, levels, internalformat, dims.x);
}

template<>
void
Impl::glTextureDims<2>::storage(const GLsizei levels, const GLenum internalformat, glm::vec<2, GLsizei> dims)
{
	glTextureStorage2D(name, levels, internalformat, dims.x, dims.y);
}

template<>
void
Impl::glTextureDims<3>::storage(const GLsizei levels, const GLenum internalformat, glm::vec<3, GLsizei> dims)
{
	glTextureStorage3D(name, levels, internalformat, dims.x, dims.y, dims.z);
}

template<>
void
Impl::glTextureDims<1>::subImage(
		glm::vec<1, GLint> offset, glm::vec<1, GLint> size, const GLenum format, const GLenum type, const void * pixels)
{
	glTextureSubImage1D(name, 0, offset.x, size.x, format, type, pixels);
}

template<>
void
Impl::glTextureDims<2>::subImage(
		glm::vec<2, GLint> offset, glm::vec<2, GLint> size, const GLenum format, const GLenum type, const void * pixels)
{
	glTextureSubImage2D(name, 0, offset.x, offset.y, size.x, size.y, format, type, pixels);
}

template<>
void
Impl::glTextureDims<3>::subImage(
		glm::vec<3, GLint> offset, glm::vec<3, GLint> size, const GLenum format, const GLenum type, const void * pixels)
{
	glTextureSubImage3D(name, 0, offset.x, offset.y, offset.z, size.x, size.y, size.z, format, type, pixels);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::image(
		glm::vec<Dims, GLint> size, const GLenum format, const GLenum type, const void * pixels)
{
	subImage({}, size, format, type, pixels);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::image(const GLenum format, const GLenum type, const void * pixels)
{
	image(getSize(), format, type, pixels);
}

namespace {
	template<glm::length_t Dims>
		requires(Dims >= 1 && Dims <= 3)
	void
	save(const Impl::glTextureDims<Dims> & texture, const GLenum format, const GLenum type, uint8_t channels,
			const char * path, uint8_t tgaFormat)
	{
		const auto size = texture.getSize();
		const auto area = [size]() {
			size_t area = 1;
			for (auto dim = 0; dim < Dims; ++dim) {
				area *= static_cast<size_t>(size[dim]);
			}
			return area;
		}();
		size_t dataSize = area * channels;
		const size_t fileSize = dataSize + sizeof(TGAHead);

		filesystem::fh out {path, O_RDWR | O_CREAT, 0660};
		out.truncate(fileSize);
		auto tga = out.mmap(fileSize, 0, PROT_WRITE, MAP_SHARED);
		*tga.get<TGAHead>() = {
				.format = tgaFormat,
				.size = {size.x, 1 * (area / static_cast<size_t>(size.x))},
				.pixelDepth = static_cast<uint8_t>(8 * channels),
		};
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetTextureImage(texture, 0, format, type, static_cast<GLsizei>(dataSize), tga.get<TGAHead>() + 1);
		tga.msync(MS_ASYNC);
	}
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::saveColour(const char * path) const
{
	save(*this, GL_BGR, GL_UNSIGNED_BYTE, 3, path, 2);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::savePosition(const char * path) const
{
	save(*this, GL_BGR_INTEGER, GL_UNSIGNED_BYTE, 3, path, 2);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::saveDepth(const char * path) const
{
	save(*this, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 1, path, 3);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::saveNormal(const char * path) const
{
	save(*this, GL_BGR, GL_BYTE, 3, path, 2);
}

template struct Impl::glTextureDims<1>;
template struct Impl::glTextureDims<2>;
template struct Impl::glTextureDims<3>;
