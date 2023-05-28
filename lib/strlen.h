#pragma once

#include <cstddef>
#include <glad/gl.h>

constexpr auto
constexpr_strlen(const GLchar * const s)
{
	std::size_t ch {};
	while (s[ch]) {
		ch++;
	}
	return ch;
}
