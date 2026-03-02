#pragma once

#include "config/types.h"
#include "glArrays.h"

namespace Impl {
	// NOLINTNEXTLINE(readability-identifier-naming)
	struct glTexture : Detail::glNamed {
		[[nodiscard]] TextureDimensions getSize() const;
		void bind(GLenum type = GL_TEXTURE_2D, GLenum unit = GL_TEXTURE0) const;
	};
}

// NOLINTBEGIN(readability-identifier-naming)
template<size_t N> using glTextures = glManagedArray<Impl::glTexture, N, &glGenTextures, &glDeleteTextures>;
using glTexture = glManagedSingle<Impl::glTexture, &glGenTextures, &glDeleteTextures>;
// NOLINTEND(readability-identifier-naming)
