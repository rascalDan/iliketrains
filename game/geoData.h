#pragma once

#include "collections.h" // IWYU pragma: keep IterableCollection
#include "config/types.h"
#include "ray.h"
#include "surface.h"
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

	template<glm::length_t Dim> struct Triangle : public glm::vec<3, glm::vec<Dim, GlobalDistance>> {
		using Point = glm::vec<Dim, GlobalDistance>;
		using base = glm::vec<3, Point>;
		using base::base;

		template<IterableCollection Range> Triangle(const GeoData * m, Range range)
		{
			assert(std::distance(range.begin(), range.end()) == 3);
			std::transform(range.begin(), range.end(), &base::operator[](0), [m](auto vh) {
				return m->point(vh);
			});
		}

		[[nodiscard]] Point
		operator*(BaryPosition bari) const
		{
			return p(0) + (::difference(p(1), p(0)) * bari.x) + (::difference(p(2), p(0)) * bari.y);
		}

		[[nodiscard]] Point
		centroid() const
		{
			return [this]<glm::length_t... axis>(std::integer_sequence<glm::length_t, axis...>) {
				return Point {(p(0)[axis] + p(1)[axis] + p(2)[axis]) / 3 ...};
			}(std::make_integer_sequence<glm::length_t, Dim>());
		}

		[[nodiscard]] auto
		area() const
			requires(Dim == 3)
		{
			return glm::length(crossProduct(::difference(p(1), p(0)), ::difference(p(2), p(0)))) / 2.F;
		}

		[[nodiscard]] Normal3D
		normal() const
			requires(Dim == 3)
		{
			return crossProduct(::difference(p(1), p(0)), ::difference(p(2), p(0)));
		}

		[[nodiscard]] Normal3D
		nnormal() const
			requires(Dim == 3)
		{
			return glm::normalize(normal());
		}

		[[nodiscard]] auto
		angle(glm::length_t c) const
		{
			return Arc {P(c), P(c + 2), P(c + 1)}.length();
		}

		template<glm::length_t D = Dim>
		[[nodiscard]] auto
		angleAt(const GlobalPosition<D> pos) const
			requires(D <= Dim)
		{
			for (glm::length_t i {}; i < 3; ++i) {
				if (GlobalPosition<D> {p(i)} == pos) {
					return angle(i);
				}
			}
			return 0.F;
		}

		[[nodiscard]] inline auto
		p(const glm::length_t i) const
		{
			return base::operator[](i);
		}

		[[nodiscard]] inline auto
		P(const glm::length_t i) const
		{
			return base::operator[](i % 3);
		}
	};

	[[nodiscard]] FaceHandle findPoint(GlobalPosition2D) const;
	[[nodiscard]] FaceHandle findPoint(GlobalPosition2D, FaceHandle start) const;

	[[nodiscard]] GlobalPosition3D positionAt(const PointFace &) const;
	using IntersectionLocation = std::pair<GlobalPosition3D, FaceHandle>;
	using IntersectionResult = std::optional<IntersectionLocation>;
	[[nodiscard]] IntersectionResult intersectRay(const Ray<GlobalPosition3D> &) const;
	[[nodiscard]] IntersectionResult intersectRay(const Ray<GlobalPosition3D> &, FaceHandle start) const;

	void walk(const PointFace & from, const GlobalPosition2D to, const std::function<void(FaceHandle)> & op) const;
	void walkUntil(const PointFace & from, const GlobalPosition2D to, const std::function<bool(FaceHandle)> & op) const;

	void boundaryWalk(const std::function<void(HalfedgeHandle)> &) const;
	void boundaryWalk(const std::function<void(HalfedgeHandle)> &, HalfedgeHandle start) const;
	void boundaryWalkUntil(const std::function<bool(HalfedgeHandle)> &) const;
	void boundaryWalkUntil(const std::function<bool(HalfedgeHandle)> &, HalfedgeHandle start) const;

	[[nodiscard]] HalfedgeHandle findEntry(const GlobalPosition2D from, const GlobalPosition2D to) const;

	struct SetHeightsOpts {
		static constexpr auto DEFAULT_NEAR_NODE_TOLERANACE = 500.F;
		static constexpr auto DEFAULT_MAX_SLOPE = 0.5F;

		const Surface & surface;
		RelativeDistance nearNodeTolerance = DEFAULT_NEAR_NODE_TOLERANACE;
		RelativeDistance maxSlope = DEFAULT_MAX_SLOPE;
	};

	void setHeights(std::span<const GlobalPosition3D> triangleStrip, const SetHeightsOpts &);

	[[nodiscard]] auto
	getExtents() const
	{
		return std::tie(lowerExtent, upperExtent);
	}

	template<typename HandleT>
	[[nodiscard]] auto
	get_surface(const HandleT h)
	{
		return property(surface, h);
	}

protected:
	template<glm::length_t Dim>
	[[nodiscard]] Triangle<Dim>
	triangle(FaceHandle f) const
	{
		return {this, fv_range(f)};
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
};
