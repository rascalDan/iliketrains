#pragma once

#include <OpenMesh/Core/Mesh/Traits.hh>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace glm {
	template<length_t L, typename T, qualifier Q>
	auto
	norm(const vec<L, T, Q> & v)
	{
		return length(v);
	}

	template<length_t L, typename T, qualifier Q, typename S>
	auto
	vectorize(vec<L, T, Q> & v, S scalar)
	{
		v = vec<L, T, Q> {static_cast<T>(scalar)};
	}
}

namespace OpenMesh {
	template<glm::length_t L, typename T, glm::qualifier Q> struct vector_traits<glm::vec<L, T, Q>> {
		using vector_type = glm::vec<L, T, Q>;
		using value_type = T;
		static constexpr glm::length_t size_ = L;
	};
}
