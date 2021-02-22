#ifndef STREAM_SUPPORT_H
#define STREAM_SUPPORT_H

#include <glm/glm.hpp>
#include <iostream>
#include <maths.h>

namespace std {
	template<glm::length_t L, glm::length_t R, typename T, glm::qualifier Q>
	std::ostream &
	operator<<(std::ostream & s, const glm::mat<L, R, T, Q> & m)
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

	template<glm::length_t L, typename T, glm::qualifier Q>
	std::ostream &
	operator<<(std::ostream & s, const glm::vec<L, T, Q> & v)
	{
		for (int x = 0; x < L; x++) {
			s << v[x] << ", ";
		}
		return s;
	}

	inline std::ostream &
	operator<<(std::ostream & s, const Arc & arc)
	{
		return s << arc.first << " ↺ " << arc.second;
	}
}

#endif
