#pragma once

#include "config/types.h"
#include "modelFactoryMesh_fwd.h"
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <glad/gl.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <thirdparty/openmesh/glmcompat.h>
#include <thirdparty/openmesh/helpers.h>

struct ModelFactoryTraits : public OpenMesh::DefaultTraits {
	FaceAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status | OpenMesh::Attributes::Color);
	EdgeAttributes(OpenMesh::Attributes::Status);
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	HalfedgeAttributes(OpenMesh::Attributes::TexCoord2D);
	using Point = RelativePosition3D;
	using Normal = Normal3D;
	using Color = RGBA;
	using TexCoord2D = TextureRelCoord;
};

struct ModelFactoryMesh : public OpenMesh::PolyMesh_ArrayKernelT<ModelFactoryTraits> {
	bool normalsProvidedProperty {};
	const OpenMesh::Helpers::Property<bool, OpenMesh::FPropHandleT> smoothFaceProperty {this};
	const OpenMesh::Helpers::Property<GLuint, OpenMesh::FPropHandleT> materialFaceProperty {this};
	const OpenMesh::Helpers::Property<std::string, OpenMesh::FPropHandleT> nameFaceProperty {this};
	const OpenMesh::Helpers::Property<std::string, OpenMesh::HPropHandleT> nameAdjFaceProperty {this};

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
