#pragma once
#ifndef GLDEBUG
#	define GLDEBUG 0
#endif

#include "special_members.h"
#include <glad/gl.h>
#include <source_location>
#include <string_view>

class [[nodiscard]] glDebugScope {
public:
	explicit glDebugScope(GLuint id, const std::source_location & = std::source_location::current());
	explicit glDebugScope(
			GLuint id, std::string_view msg, const std::source_location & = std::source_location::current());

	~glDebugScope();

	constexpr
	operator bool() const
	{
		return true;
	}

	NO_MOVE(glDebugScope);
	NO_COPY(glDebugScope);
};

#if GLDEBUG > 0
inline glDebugScope::~glDebugScope()
{
	glPopDebugGroup();
}
#	if GLDEBUG == 1
// Level 1 is inlined for performance because they're thin wrappers
inline glDebugScope::glDebugScope(GLuint id, const std::source_location & location) :
	glDebugScope {id, location.function_name()}
{
}

inline glDebugScope::glDebugScope(GLuint id, const std::string_view msg, const std::source_location &)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<GLsizei>(msg.length()), msg.data());
}
#	endif
#else
inline glDebugScope::glDebugScope(GLuint, const std::source_location &) { }

inline glDebugScope::glDebugScope(GLuint, const std::string_view, const std::source_location &) { }

inline glDebugScope::~glDebugScope() = default;
#endif
