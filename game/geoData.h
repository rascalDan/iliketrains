#pragma once

#include "collections.h" // IWYU pragma: keep IterableCollection
#include "ray.h"
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <filesystem>
#include <glm/vec2.hpp>
#include <optional>
#include <thirdparty/openmesh/glmcompat.h>

struct GeoDataTraits : public OpenMesh::DefaultTraits {
	FaceAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
	VertexAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	HalfedgeAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
	using Point = glm::vec3;
	using Normal = glm::vec3;
};

class GeoData : public OpenMesh::TriMesh_ArrayKernelT<GeoDataTraits> {
private:
	GeoData() = default;

public:
	static GeoData loadFromAsciiGrid(const std::filesystem::path &);
	static GeoData createFlat(glm::vec2 lower, glm::vec2, float h);

	struct PointFace {
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		PointFace(const glm::vec2 p) : point {p} { }

		PointFace(const glm::vec2 p, FaceHandle face) : point {p}, _face {face} { }

		PointFace(const glm::vec2 p, const GeoData *);
		PointFace(const glm::vec2 p, const GeoData *, FaceHandle start);

		const glm::vec2 point;
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

	template<glm::length_t Dim> struct Triangle : public glm::vec<3, glm::vec<Dim, glm::vec2::value_type>> {
		using base = glm::vec<3, glm::vec<Dim, glm::vec2::value_type>>;
		using base::base;

		template<IterableCollection Range> Triangle(const GeoData * m, Range range)
		{
			assert(std::distance(range.begin(), range.end()) == 3);
			std::transform(range.begin(), range.end(), &base::operator[](0), [m](auto vh) {
				return m->point(vh);
			});
		}

		glm::vec<Dim, glm::vec2::value_type>
		operator*(glm::vec2 bari) const
		{
			const auto & t {*this};
			return t[0] + ((t[1] - t[0]) * bari.x) + ((t[2] - t[1]) * bari.y);
		}
	};

	[[nodiscard]] FaceHandle findPoint(glm::vec2) const;
	[[nodiscard]] FaceHandle findPoint(glm::vec2, FaceHandle start) const;

	[[nodiscard]] glm::vec3 positionAt(const PointFace &) const;
	[[nodiscard]] std::optional<glm::vec3> intersectRay(const Ray &) const;
	[[nodiscard]] std::optional<glm::vec3> intersectRay(const Ray &, FaceHandle start) const;

	void walk(const PointFace & from, const glm::vec2 to, const std::function<void(FaceHandle)> & op) const;
	void walkUntil(const PointFace & from, const glm::vec2 to, const std::function<bool(FaceHandle)> & op) const;

	void boundaryWalk(const std::function<void(HalfedgeHandle)> &) const;
	void boundaryWalk(const std::function<void(HalfedgeHandle)> &, HalfedgeHandle start) const;
	void boundaryWalkUntil(const std::function<bool(HalfedgeHandle)> &) const;
	void boundaryWalkUntil(const std::function<bool(HalfedgeHandle)> &, HalfedgeHandle start) const;

	[[nodiscard]] auto
	getExtents() const
	{
		return std::tie(lowerExtent, upperExtent);
	}

protected:
	template<glm::length_t Dim>
	[[nodiscard]] Triangle<Dim>
	triangle(FaceHandle f) const
	{
		return {this, fv_range(f)};
	}

	[[nodiscard]] static bool triangleContainsPoint(const glm::vec2, const Triangle<2> &);
	[[nodiscard]] bool triangleContainsPoint(const glm::vec2, FaceHandle) const;
	[[nodiscard]] HalfedgeHandle findBoundaryStart() const;

private:
	glm::vec3 lowerExtent {}, upperExtent {};
};
