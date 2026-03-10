#pragma once

#include "config/types.h"
#include "glArrays.h"
#include "glTexture.h"

namespace Impl {
	struct glRenderbuffer : Detail::glNamed {
		void storage(GLenum iformat, ImageDimensions);
	};

	struct glFramebuffer : Detail::glNamed {
		template<GLenum Target>
		void
		texture(GLenum attachment, const glTexture<Target> & texture)
		{
			glNamedFramebufferTexture(name, attachment, texture, 0);
		}

		void drawBuffers(std::span<const GLenum> buffers);

		template<std::convertible_to<GLenum>... Buffers>
		void
		drawBuffers(Buffers... buffers)
		{
			drawBuffers(std::array {static_cast<GLenum>(buffers)...});
		}

		void buffer(GLenum attachment, const glRenderbuffer &);
		void assertComplete() const;
	};
}

template<size_t N>
using glFramebuffers = glManagedArray<Impl::glFramebuffer, N, &glCreateFramebuffers, &glDeleteFramebuffers>;
using glFramebuffer = glManagedSingle<Impl::glFramebuffer, &glCreateFramebuffers, &glDeleteFramebuffers>;

template<size_t N>
using glRenderbuffers = glManagedArray<Impl::glRenderbuffer, N, &glCreateRenderbuffers, &glDeleteRenderbuffers>;
using glRenderbuffer = glManagedSingle<Impl::glRenderbuffer, &glCreateRenderbuffers, &glDeleteRenderbuffers>;
