#pragma once

#include "config/types.h"
#include "triangle.h"
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <source_location>
#include <thirdparty/openmesh/glmcompat.h>
#include <thirdparty/openmesh/helpers.h>

struct GeoDataTraits : public OpenMesh::DefaultTraits {
	FaceAttributes(OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	HalfedgeAttributes(OpenMesh::Attributes::Status);
	using Point = GlobalPosition3D;
	using Normal = Normal3D;
};

class GeoDataMesh : public OpenMesh::TriMesh_ArrayKernelT<GeoDataTraits> {
public:
	struct PointFace {
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		PointFace(GlobalPosition2D coord) : point {coord} { }

		PointFace(GlobalPosition2D coord, FaceHandle face) : point {coord}, faceCache {face} { }

		PointFace(GlobalPosition2D coord, const GeoDataMesh *);
		PointFace(GlobalPosition2D coord, GeoDataMesh const *, FaceHandle start);

		const GlobalPosition2D point;
		[[nodiscard]] FaceHandle face(const GeoDataMesh *) const;
		[[nodiscard]] FaceHandle face(const GeoDataMesh *, FaceHandle start) const;

		[[nodiscard]] bool
		isLocated() const
		{
			return faceCache.is_valid();
		}

	private:
		mutable FaceHandle faceCache;
	};

	template<glm::length_t Dim> using Triangle = ::Triangle<Dim, GlobalDistance>;

	[[nodiscard]] FaceHandle findPoint(GlobalPosition2D) const;
	[[nodiscard]] FaceHandle findPoint(GlobalPosition2D, FaceHandle) const;

	[[nodiscard]] GlobalPosition3D positionAt(const PointFace &) const;

protected:
	void sanityCheck(const std::source_location & = std::source_location::current()) const;

	[[nodiscard]] bool faceContainsPoint(GlobalPosition2D, FaceHandle) const;
	[[nodiscard]] HalfedgeHandle findBoundaryStart() const;
	[[nodiscard]] RelativePosition3D difference(HalfedgeHandle) const;
	using HalfEdgeVertices = std::pair<VertexHandle, VertexHandle>;
	[[nodiscard]] HalfEdgeVertices toVertexHandles(HalfedgeHandle) const;
	using HalfEdgePoints = std::pair<GlobalPosition3D, GlobalPosition3D>;
	[[nodiscard]] HalfEdgePoints points(HalfEdgeVertices) const;

	template<glm::length_t D>
	[[nodiscard]] auto
	vertexDistanceFunction(GlobalPosition<D> point) const
	{
		struct DistanceCalculator {
			[[nodiscard]] std::pair<VertexHandle, float>
			operator()(VertexHandle compVertex) const
			{
				return std::make_pair(
						compVertex, ::distance<D, GlobalDistance, glm::defaultp>(point, mesh->point(compVertex)));
			}

			[[nodiscard]]
			std::pair<HalfedgeHandle, float>
			operator()(const HalfedgeHandle compHalfedge) const
			{
				const auto edgePoints = mesh->points(mesh->toVertexHandles(compHalfedge));
				return std::make_pair(compHalfedge, Triangle<2> {edgePoints.second, edgePoints.first, point}.height());
			};

			const GeoDataMesh * mesh;
			GlobalPosition<D> point;
		};

		return DistanceCalculator {this, point};
	}

	[[nodiscard]] bool canFlip(HalfedgeHandle edge) const;
	[[nodiscard]] std::optional<EdgeHandle> shouldFlip(HalfedgeHandle next, GlobalPosition2D startPoint) const;

	template<glm::length_t D>
	[[nodiscard]] RelativeDistance
	length(HalfedgeHandle heh) const
	{
		return ::distance<D, GlobalDistance, glm::defaultp>(
				point(to_vertex_handle(heh)), point(from_vertex_handle(heh)));
	}

	[[nodiscard]] GlobalPosition3D centre(HalfedgeHandle) const;

	template<glm::length_t Dim>
	[[nodiscard]] Triangle<Dim>
	triangle(FaceHandle face) const
	{
		Triangle<Dim> triangle;
		std::ranges::transform(fv_range(face), triangle.begin(), [this](auto vertex) {
			return point(vertex);
		});
		return triangle;
	}
};
