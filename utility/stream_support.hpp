#ifndef STREAM_SUPPORT_H
#define STREAM_SUPPORT_H

#include <iostream>

template<glm::length_t L, glm::length_t R, typename T, glm::qualifier Q>
std::ostream &
operator<<(std::ostream & s, glm::mat<L, R, T, Q> & m)
{
	for (int y = 0; y < m.length(); y++) {
		const auto & col = m[y];
		for (int x = 0; x < col.length(); x++) {
			s << col[x] << ", ";
		}
		s << "\n";
	}
	return s;
}

#endif
