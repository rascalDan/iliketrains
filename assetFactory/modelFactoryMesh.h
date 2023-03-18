#pragma once

#include "modelFactoryMesh_fwd.h"
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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

struct ModelFactoryTraits : public OpenMesh::DefaultTraits {
	FaceAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status | OpenMesh::Attributes::Color);
	EdgeAttributes(OpenMesh::Attributes::Status);
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	HalfedgeAttributes(OpenMesh::Attributes::TexCoord2D);
	using Point = glm::vec3;
	using Normal = glm::vec3;
	using Color = glm::vec4;
	using TexCoord2D = glm::vec2;
};

struct ModelFactoryMesh : public OpenMesh::PolyMesh_ArrayKernelT<ModelFactoryTraits> {
	ModelFactoryMesh();

	OpenMesh::FPropHandleT<bool> smoothFaceProperty;
	OpenMesh::FPropHandleT<std::string> nameFaceProperty;

	template<typename... Vs>
	std::pair<std::string, OpenMesh::FaceHandle>
	add_namedFace(std::string name, Vs &&... vs)
	{
		const auto handle = add_face(std::forward<Vs>(vs)...);
		property(nameFaceProperty, handle) = name;
		return std::make_pair(name, handle);
	}
};
