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

	struct PointFace {
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		PointFace(const glm::vec2 p) : point {p} { }

		PointFace(const glm::vec2 p, FaceHandle face) : point {p}, _face {face} { }

		PointFace(const glm::vec2 p, const TerrainMesh *);
		PointFace(const glm::vec2 p, const TerrainMesh *, FaceHandle start);

		const glm::vec2 point;
		[[nodiscard]] FaceHandle face(const TerrainMesh *) const;
		[[nodiscard]] FaceHandle face(const TerrainMesh *, FaceHandle start) const;

		[[nodiscard]] bool
		isLocated() const
		{
			return _face.is_valid();
		}

	private:
		mutable FaceHandle _face {};
	};

	[[nodiscard]] FaceHandle findPoint(glm::vec2) const;
	[[nodiscard]] FaceHandle findPoint(glm::vec2, FaceHandle start) const;

	void walk(const PointFace & from, const glm::vec2 to, const std::function<void(FaceHandle)> & op) const;
	void walkUntil(const PointFace & from, const glm::vec2 to, const std::function<bool(FaceHandle)> & op) const;

protected:
	[[nodiscard]] static bool triangleContainsPoint(const glm::vec2, const glm::vec2, const glm::vec2, const glm::vec2);
	[[nodiscard]] bool triangleContainsPoint(const glm::vec2, FaceHandle) const;
	[[nodiscard]] bool triangleContainsPoint(const glm::vec2, ConstFaceVertexIter) const;
};
