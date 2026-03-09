#include "glTexture.h"
#include "config/types.h"
#include "filesystem.h"
#include "gfx/models/tga.h"
#include "maths.h"
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
	size_t
	areaOf(glm::vec<Dims, GLsizei> size)
	{
		size_t area = 1;
		for (auto dim = 0; dim < Dims; ++dim) {
			area *= static_cast<size_t>(size[dim]);
		}
		return area;
	}

	template<glm::length_t Dims, glm::length_t channels>
		requires(Dims >= 1 && Dims <= 3)
	void
	save(const Impl::glTextureDims<Dims> & texture, const GLenum format, const GLenum type, const char * path,
			uint8_t tgaFormat)
	{
		const auto size = texture.getSize();
		const auto area = areaOf(size);
		size_t dataSize = area * channels;
		const size_t fileSize = dataSize + sizeof(TGAHead<channels>);

		filesystem::fh out {path, O_RDWR | O_CREAT, 0660};
		out.truncate(fileSize);
		auto tga = out.mmap(fileSize, 0, PROT_WRITE, MAP_SHARED);
		auto outTga = tga.get<TGAHead<channels>>();
		*outTga = {
				.format = tgaFormat,
				.size = {size.x, (area / static_cast<size_t>(size.x))},
		};
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetTextureImage(texture, 0, format, type, static_cast<GLsizei>(dataSize), outTga->data);
		tga.msync(MS_ASYNC);
	}
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::saveColour(const char * path) const
{
	save<Dims, 3>(*this, GL_BGR, GL_UNSIGNED_BYTE, path, 2);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::savePosition(const char * path) const
{
	const auto size = getSize();
	const auto area = areaOf(size);
	size_t dataSize = area * sizeof(TGAHead<3>::PixelType);
	const size_t fileSize = dataSize + sizeof(TGAHead<3>);

	filesystem::fh out {path, O_RDWR | O_CREAT, 0660};
	out.truncate(fileSize);
	auto tga = out.mmap(fileSize, 0, PROT_WRITE, MAP_SHARED);
	auto outTga = tga.get<TGAHead<3>>();
	*outTga = {
			.format = 2,
			.size = {size.x, (area / static_cast<size_t>(size.x))},
	};
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	std::vector<GlobalPosition3D> raw {area};
	glGetTextureImage(
			name, 0, GL_BGR_INTEGER, GL_INT, static_cast<GLsizei>(sizeof(GlobalPosition3D) * area), raw.data());
	using Comp = GlobalPosition3D (*)(const GlobalPosition3D &, const GlobalPosition3D &);
	auto notZero = std::views::filter([](const GlobalPosition3D & pos) {
		return pos != GlobalPosition3D {};
	});
	const auto minPos = *std::ranges::fold_left_first(raw | notZero, static_cast<Comp>(&glm::min));
	const auto maxPos = *std::ranges::fold_left_first(raw | notZero, static_cast<Comp>(&glm::max));
	const auto rangePos = difference(maxPos, minPos);
	std::ranges::transform(raw, outTga->data, [minPos, rangePos](const GlobalPosition3D & pos) {
		return GlobalPosition3D(255.F * (difference(pos, minPos) / rangePos));
	});
	tga.msync(MS_ASYNC);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::saveDepth(const char * path) const
{
	save<Dims, 1>(*this, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, path, 3);
}

template<glm::length_t Dims>
	requires(Dims >= 1 && Dims <= 3)
void
Impl::glTextureDims<Dims>::saveNormal(const char * path) const
{
	save<Dims, 3>(*this, GL_BGR, GL_BYTE, path, 2);
}

template struct Impl::glTextureDims<1>;
template struct Impl::glTextureDims<2>;
template struct Impl::glTextureDims<3>;
