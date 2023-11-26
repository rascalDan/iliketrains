#pragma once

#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Utils/vector_cast.hh>
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
	template<typename dst_t, typename src_t, glm::length_t n, glm::qualifier Q>
	struct vector_caster<glm::vec<n, dst_t, Q>, glm::vec<n, src_t, Q>> {
		using source_type = glm::vec<n, src_t, Q>;
		using return_type = glm::vec<n, dst_t, Q>;

		inline static return_type
		cast(const source_type & _src)
		{
			return return_type {_src};
		}
	};

	template<glm::length_t L, typename T, glm::qualifier Q> struct vector_traits<glm::vec<L, T, Q>> {
		using vector_type = glm::vec<L, T, Q>;
		using value_type = T;
		static constexpr glm::length_t size_ = L;
	};
}
