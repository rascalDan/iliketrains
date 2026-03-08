#pragma once

#include "glArrays.h"

namespace Impl {
	struct glBuffer : Detail::glNamed {
		void
		storage(const std::ranges::contiguous_range auto & data, GLenum flags)
		{
			glNamedBufferStorage(
					name, static_cast<GLsizeiptr>(data.size() * sizeof(decltype(*data.data()))), data.data(), flags);
		}

		void
		data(const std::ranges::contiguous_range auto & data, GLenum flags)
		{
			glNamedBufferData(
					name, static_cast<GLsizeiptr>(data.size() * sizeof(decltype(*data.data()))), data.data(), flags);
		}
	};
}

template<size_t N> using glBuffers = glManagedArray<Impl::glBuffer, N, &glCreateBuffers, &glDeleteBuffers>;
using glBuffer = glManagedSingle<Impl::glBuffer, &glCreateBuffers, &glDeleteBuffers>;
