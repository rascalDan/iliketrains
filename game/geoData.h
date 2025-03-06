#pragma once

#include "collections.h" // IWYU pragma: keep IterableCollection
#include "geoDataMesh.h"
#include "gfx/aabb.h"
#include "surface.h"
#include <filesystem>
#include <glm/vec2.hpp>

class GeoData : public GeoDataMesh {
private:
	const OpenMesh::Helpers::Property<const Surface *, OpenMesh::FPropHandleT> surface {this};

public:
	static GeoData loadFromAsciiGrid(const std::filesystem::path &);
	static GeoData createFlat(GlobalPosition2D lower, GlobalPosition2D upper, GlobalDistance h);

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

	struct WalkStepCurve : public WalkStep {
		Angle angle {};
	};

	template<typename T> using Consumer = const std::function<void(const T &)> &;
	template<typename T> using Tester = const std::function<bool(const T &)> &;

	void walk(const PointFace & from, GlobalPosition2D to, Consumer<WalkStep> op) const;
	void walkUntil(const PointFace & from, GlobalPosition2D to, Tester<WalkStep> op) const;
	void walk(const PointFace & from, GlobalPosition2D to, GlobalPosition2D centre, Consumer<WalkStepCurve> op) const;
	void walkUntil(
			const PointFace & from, GlobalPosition2D to, GlobalPosition2D centre, Tester<WalkStepCurve> op) const;

	void boundaryWalk(Consumer<HalfedgeHandle>) const;
	void boundaryWalk(Consumer<HalfedgeHandle>, HalfedgeHandle start) const;
	void boundaryWalkUntil(Tester<HalfedgeHandle>) const;
	void boundaryWalkUntil(Tester<HalfedgeHandle>, HalfedgeHandle start) const;

	[[nodiscard]] HalfedgeHandle findEntry(GlobalPosition2D from, GlobalPosition2D to) const;

	struct SetHeightsOpts {
		static constexpr auto DEFAULT_NEAR_NODE_TOLERANACE = 500.F;
		static constexpr auto DEFAULT_MAX_SLOPE = 0.5F;

		const Surface * surface = nullptr;
		RelativeDistance nearNodeTolerance = DEFAULT_NEAR_NODE_TOLERANACE;
		RelativeDistance maxSlope = DEFAULT_MAX_SLOPE;
	};

	std::set<FaceHandle> setHeights(std::span<const GlobalPosition3D> triangleStrip, const SetHeightsOpts &);

	[[nodiscard]] auto &
	getExtents() const
	{
		return extents;
	}

	template<typename HandleT>
		requires(std::derived_from<HandleT, OpenMesh::BaseHandle>)
	[[nodiscard]] auto
	getSurface(const HandleT handle) const
	{
		assert(handle.is_valid());
		return property(surface, handle);
	}

protected:
	[[nodiscard]] VertexHandle setPoint(GlobalPosition3D point, RelativeDistance nearNodeTolerance);
	void updateAllVertexNormals();
	template<std::ranges::range R> void updateAllVertexNormals(const R &);
	void updateVertexNormal(VertexHandle);
	virtual void afterChange();

private:
	AxisAlignedBoundingBox extents;
};
