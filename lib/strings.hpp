#ifndef STRINGS_HPP
#define STRINGS_HPP

constexpr auto
constexpr_strlen(const GLchar * const s)
{
	std::size_t ch {};
	while (s[ch]) {
		ch++;
	}
	return ch;
}

#endif
