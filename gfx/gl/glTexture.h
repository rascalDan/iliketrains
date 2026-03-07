#pragma once

#include "config/types.h"
#include "glArrays.h"
#include "gl_traits.h"

namespace Impl {
	// NOLINTNEXTLINE(readability-identifier-naming)
	struct glTexture : Detail::glNamed {
		[[nodiscard]] TextureDimensions getSize() const;
		void bind(GLenum type = GL_TEXTURE_2D, GLenum unit = GL_TEXTURE0) const;

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
}

// NOLINTBEGIN(readability-identifier-naming)
template<size_t N> using glTextures = glManagedArray<Impl::glTexture, N, &glGenTextures, &glDeleteTextures>;
using glTexture = glManagedSingle<Impl::glTexture, &glGenTextures, &glDeleteTextures>;
// NOLINTEND(readability-identifier-naming)
