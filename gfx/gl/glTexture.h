#pragma once

#include "glArrays.h"
#include "gl_traits.h"

namespace Impl {
	template<GLenum> struct TextureTargetTraits;

	template<GLenum Target> struct TextureTargetTraitsCommon {
		static void
		create(GLsizei count, GLuint * textures)
		{
			glCreateTextures(Target, count, textures);
		}
	};

	template<> struct TextureTargetTraits<GL_TEXTURE_2D> : TextureTargetTraitsCommon<GL_TEXTURE_2D> {
		constexpr static glm::length_t dims = 2;
	};

	template<> struct TextureTargetTraits<GL_TEXTURE_2D_ARRAY> : TextureTargetTraitsCommon<GL_TEXTURE_2D_ARRAY> {
		constexpr static glm::length_t dims = 3;
	};

	template<> struct TextureTargetTraits<GL_TEXTURE_RECTANGLE> : TextureTargetTraitsCommon<GL_TEXTURE_RECTANGLE> {
		constexpr static glm::length_t dims = 2;
	};

	struct glTextureBase : Detail::glNamed {
		void bind(GLuint unit) const;
		void generateMipmap();

		template<has_glTextureParameter T>
		void
		parameter(GLenum pname, T param)
		{
			(*gl_traits<T>::glTextureParameterFunc)(name, pname, param);
		}

		template<glm::length_t L, has_glTextureParameterv T, glm::qualifier Q>
		void
		parameter(GLenum pname, const glm::vec<L, T, Q> & param)
		{
			(*gl_traits<T>::glTextureParametervFunc)(name, pname, glm::value_ptr(param));
		}
	};

	template<glm::length_t Dims>
		requires(Dims >= 1 && Dims <= 3)
	struct glTextureDims : glTextureBase {
		[[nodiscard]] glm::vec<Dims, GLsizei> getSize() const;
		void storage(GLsizei levels, GLenum internalformat, glm::vec<Dims, GLsizei> dims);
		void image(GLenum format, GLenum type, const void * pixels);
		void image(glm::vec<Dims, GLint> size, GLenum format, GLenum type, const void * pixels);
		void subImage(glm::vec<Dims, GLint> offset, glm::vec<Dims, GLint> size, GLenum format, GLenum type,
				const void * pixels);

		void saveColour(const char * path) const;
		void saveDepth(const char * path) const;
		void saveNormal(const char * path) const;
		void savePosition(const char * path) const;
	};

	template<GLenum Target> struct glTexture : glTextureDims<TextureTargetTraits<Target>::dims> { };
}

template<GLenum Target, size_t N>
using glTextures
		= glManagedArray<Impl::glTexture<Target>, N, &Impl::TextureTargetTraits<Target>::create, &glDeleteTextures>;
template<GLenum Target>
using glTexture
		= glManagedSingle<Impl::glTexture<Target>, &Impl::TextureTargetTraits<Target>::create, &glDeleteTextures>;
