#if GLDEBUG == 2
// Level 2 is out of line because its "complex"

#	include "gldebug.h"
#	include <format>

glDebugScope::glDebugScope(GLuint id, const std::source_location & location)
{
	const auto fullMsg = std::format("{} ({}:{})", location.function_name(), location.file_name(), location.line());
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<GLsizei>(fullMsg.length()), fullMsg.c_str());
}

glDebugScope::glDebugScope(GLuint id, const std::string_view msg, const std::source_location & location)
{
	const auto fullMsg
			= std::format("{} @ {} ({}:{})", msg, location.function_name(), location.file_name(), location.line());
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<GLsizei>(fullMsg.length()), fullMsg.c_str());
}

#endif
