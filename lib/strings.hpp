#pragma once

#include <GL/glew.h>

constexpr auto
constexpr_strlen(const GLchar * const s)
{
	std::size_t ch {};
	while (s[ch]) {
		ch++;
	}
	return ch;
}
