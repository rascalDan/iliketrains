#pragma once

#include "config/types.h"
#include "glArrays.h"

namespace Impl {
	// NOLINTNEXTLINE(readability-identifier-naming)
	struct glVertexArray : Detail::glNamed { };
}

// NOLINTBEGIN(readability-identifier-naming)
template<size_t N>
using glVertexArrays = glManagedArray<Impl::glVertexArray, N, &glCreateVertexArrays, &glDeleteVertexArrays>;
using glVertexArray = glManagedSingle<Impl::glVertexArray, &glCreateVertexArrays, &glDeleteVertexArrays>;
// NOLINTEND(readability-identifier-naming)
