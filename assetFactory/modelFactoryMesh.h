#pragma once

#include "modelFactoryMesh_fwd.h"
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace OpenMesh {
	template<typename Scalar, int DIM> struct glmvec : public VectorT<Scalar, DIM> {
		using VectorT<Scalar, DIM>::VectorT;
		glmvec(const VectorT<Scalar, DIM> & v) : VectorT<Scalar, DIM> {v} { }
		operator glm::vec<DIM, Scalar>() const
		{
			glm::vec<DIM, Scalar> out;
			std::copy(this->begin(), this->end(), &out[0]);
			return out;
		}
	};
}

struct ModelFactoryTraits : public OpenMesh::DefaultTraits {
	FaceAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status | OpenMesh::Attributes::Color);
	EdgeAttributes(OpenMesh::Attributes::Status);
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	using Point = OpenMesh::glmvec<float, 3>;
	using Normal = OpenMesh::glmvec<float, 3>;
	using Color = glm::vec4;
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
