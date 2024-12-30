#pragma once

#include "collections.h" // IWYU pragma: keep IterableCollection
#include "config/types.h"
#include "ray.h"
#include "surface.h"
#include "triangle.h"
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <filesystem>
#include <glm/vec2.hpp>
#include <optional>
#include <thirdparty/openmesh/glmcompat.h>

struct GeoDataTraits : public OpenMesh::DefaultTraits {
	FaceAttributes(OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	HalfedgeAttributes(OpenMesh::Attributes::Status);
	using Point = GlobalPosition3D;
	using Normal = Normal3D;
};

class GeoData : public OpenMesh::TriMesh_ArrayKernelT<GeoDataTraits> {
private:
	GeoData();

	OpenMesh::FPropHandleT<const Surface *> surface;

public:
	static GeoData loadFromAsciiGrid(const std::filesystem::path &);
	static GeoData createFlat(GlobalPosition2D lower, GlobalPosition2D upper, GlobalDistance h);

	struct PointFace {
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		PointFace(const GlobalPosition2D p) : point {p} { }

		PointFace(const GlobalPosition2D p, FaceHandle face) : point {p}, _face {face} { }

		PointFace(const GlobalPosition2D p, const GeoData *);
		PointFace(const GlobalPosition2D p, const GeoData *, FaceHandle start);

		const GlobalPosition2D point;
		[[nodiscard]] FaceHandle face(const GeoData *) const;
		[[nodiscard]] FaceHandle face(const GeoData *, FaceHandle start) const;

		[[nodiscard]] bool
		isLocated() const
		{
			return _face.is_valid();
		}

	private:
		mutable FaceHandle _face {};
	};

	template<glm::length_t Dim> using Triangle = ::Triangle<Dim, GlobalDistance>;

	[[nodiscard]] FaceHandle findPoint(GlobalPosition2D) const;
	[[nodiscard]] FaceHandle findPoint(GlobalPosition2D, FaceHandle start) const;

	[[nodiscard]] GlobalPosition3D positionAt(const PointFace &) const;
	using IntersectionLocation = std::pair<GlobalPosition3D, FaceHandle>;
	using IntersectionResult = std::optional<IntersectionLocation>;
	[[nodiscard]] IntersectionResult intersectRay(const Ray<GlobalPosition3D> &) const;
	[[nodiscard]] IntersectionResult intersectRay(const Ray<GlobalPosition3D> &, FaceHandle start) const;

	struct WalkStep {
		FaceHandle current;
		FaceHandle previous {};
		HalfedgeHandle exitHalfedge {};
		GlobalPosition2D exitPosition {};
	};

	template<typename T> using Consumer = const std::function<void(const T &)> &;
	template<typename T> using Tester = const std::function<bool(const T &)> &;

	void walk(const PointFace & from, const GlobalPosition2D to, Consumer<WalkStep> op) const;
	void walkUntil(const PointFace & from, const GlobalPosition2D to, Tester<WalkStep> op) const;

	void boundaryWalk(Consumer<HalfedgeHandle>) const;
	void boundaryWalk(Consumer<HalfedgeHandle>, HalfedgeHandle start) const;
	void boundaryWalkUntil(Tester<HalfedgeHandle>) const;
	void boundaryWalkUntil(Tester<HalfedgeHandle>, HalfedgeHandle start) const;

	[[nodiscard]] HalfedgeHandle findEntry(const GlobalPosition2D from, const GlobalPosition2D to) const;

	struct SetHeightsOpts {
		static constexpr auto DEFAULT_NEAR_NODE_TOLERANACE = 500.F;
		static constexpr auto DEFAULT_MAX_SLOPE = 0.5F;

		const Surface * surface = nullptr;
		RelativeDistance nearNodeTolerance = DEFAULT_NEAR_NODE_TOLERANACE;
		RelativeDistance maxSlope = DEFAULT_MAX_SLOPE;
	};

	void setHeights(std::span<const GlobalPosition3D> triangleStrip, const SetHeightsOpts &);
	[[nodiscard]] size_t getGeneration() const;

	[[nodiscard]] auto
	getExtents() const
	{
		return std::tie(lowerExtent, upperExtent);
	}

	template<typename HandleT>
	[[nodiscard]] auto
	getSurface(const HandleT handle) const
	{
		return property(surface, handle);
	}

	void sanityCheck() const;

protected:
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

	[[nodiscard]] static bool triangleContainsPoint(const GlobalPosition2D, const Triangle<2> &);
	[[nodiscard]] bool triangleContainsPoint(const GlobalPosition2D, FaceHandle) const;
	[[nodiscard]] static bool triangleOverlapsTriangle(const Triangle<2> &, const Triangle<2> &);
	[[nodiscard]] static bool triangleContainsTriangle(const Triangle<2> &, const Triangle<2> &);
	[[nodiscard]] HalfedgeHandle findBoundaryStart() const;
	[[nodiscard]] RelativePosition3D difference(const HalfedgeHandle) const;

	[[nodiscard]] RelativeDistance length(const HalfedgeHandle) const;
	[[nodiscard]] GlobalPosition3D centre(const HalfedgeHandle) const;

	void updateAllVertexNormals();
	template<std::ranges::range R> void updateAllVertexNormals(const R &);
	void updateVertexNormal(VertexHandle);

private:
	GlobalPosition3D lowerExtent {}, upperExtent {};
	size_t generation {};
};
