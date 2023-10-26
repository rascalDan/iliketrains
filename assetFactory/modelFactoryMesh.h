#pragma once

#include "modelFactoryMesh_fwd.h"
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <glad/gl.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <thirdparty/openmesh/glmcompat.h>

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

	bool normalsProvidedProperty {};
	OpenMesh::FPropHandleT<bool> smoothFaceProperty;
	OpenMesh::FPropHandleT<GLuint> materialFaceProperty;
	OpenMesh::FPropHandleT<std::string> nameFaceProperty;
	OpenMesh::HPropHandleT<std::string> nameAdjFaceProperty;

	template<typename... Vs>
	std::pair<std::string, OpenMesh::FaceHandle>
	add_namedFace(std::string name, Vs &&... vs)
	{
		const auto handle = add_face(std::forward<Vs>(vs)...);
		configNamedFace(name, handle);
		return {std::move(name), handle};
	}

private:
	void configNamedFace(const std::string & name, OpenMesh::FaceHandle);
};
