#pragma once

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <filesystem>
#include <glm/vec2.hpp>
#include <thirdparty/openmesh/glmcompat.h>

struct TerrainTraits : public OpenMesh::DefaultTraits {
	FaceAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	HalfedgeAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	using Point = glm::vec3;
	using Normal = glm::vec3;
};

class TerrainMesh : public OpenMesh::TriMesh_ArrayKernelT<TerrainTraits> {
public:
	explicit TerrainMesh(const std::filesystem::path &);

	[[nodiscard]] FaceHandle findPoint(glm::vec2) const;
	[[nodiscard]] FaceHandle findPoint(glm::vec2, FaceHandle start) const;

protected:
	[[nodiscard]] static bool triangleContainsPoint(const glm::vec2, const glm::vec2, const glm::vec2, const glm::vec2);
	[[nodiscard]] bool triangleContainsPoint(const glm::vec2, FaceHandle) const;
	[[nodiscard]] bool triangleContainsPoint(const glm::vec2, ConstFaceVertexIter) const;
};
