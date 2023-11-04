#include "geoData.h"
#include <fstream>
#include <glm/gtx/intersect.hpp>
#include <maths.h>

GeoData
GeoData::loadFromAsciiGrid(const std::filesystem::path & input)
{
	size_t ncols = 0, nrows = 0, xllcorner = 0, yllcorner = 0, cellsize = 0;
	std::map<std::string_view, size_t *> properties {
			{"ncols", &ncols},
			{"nrows", &nrows},
			{"xllcorner", &xllcorner},
			{"yllcorner", &yllcorner},
			{"cellsize", &cellsize},
	};
	std::ifstream f {input};
	while (!properties.empty()) {
		std::string property;
		f >> property;
		f >> *properties.at(property);
		properties.erase(property);
	}
	std::vector<VertexHandle> vertices;
	vertices.reserve(ncols * nrows);
	GeoData mesh;
	mesh.lowerExtent = {xllcorner, yllcorner, std::numeric_limits<float>::max()};
	mesh.upperExtent
			= {xllcorner + (cellsize * ncols), yllcorner + (cellsize * nrows), std::numeric_limits<float>::min()};
	for (size_t row = 0; row < nrows; ++row) {
		for (size_t col = 0; col < ncols; ++col) {
			float height = 0;
			f >> height;
			mesh.upperExtent.z = std::max(mesh.upperExtent.z, height);
			mesh.lowerExtent.z = std::min(mesh.lowerExtent.z, height);
			vertices.push_back(mesh.add_vertex({xllcorner + (col * cellsize), yllcorner + (row * cellsize), height}));
		}
	}
	if (!f.good()) {
		throw std::runtime_error("Couldn't read terrain file");
	}
	for (size_t row = 1; row < nrows; ++row) {
		for (size_t col = 1; col < ncols; ++col) {
			mesh.add_face({
					vertices[ncols * (row - 1) + (col - 1)],
					vertices[ncols * (row - 0) + (col - 0)],
					vertices[ncols * (row - 0) + (col - 1)],
			});
			mesh.add_face({
					vertices[ncols * (row - 1) + (col - 1)],
					vertices[ncols * (row - 1) + (col - 0)],
					vertices[ncols * (row - 0) + (col - 0)],
			});
		}
	}
	mesh.update_face_normals();
	mesh.update_vertex_normals();

	return mesh;
};

GeoData
GeoData::createFlat(glm::vec2 lower, glm::vec2 upper, float h)
{
	GeoData mesh;

	mesh.lowerExtent = lower ^ h;
	mesh.upperExtent = upper ^ h;

	const auto ll = mesh.add_vertex({lower.x, lower.y, h}), lu = mesh.add_vertex({lower.x, upper.y, h}),
			   ul = mesh.add_vertex({upper.x, lower.y, h}), uu = mesh.add_vertex({upper.x, upper.y, h});

	mesh.add_face(ll, uu, lu);
	mesh.add_face(ll, ul, uu);

	mesh.update_face_normals();
	mesh.update_vertex_normals();

	return mesh;
}

OpenMesh::FaceHandle
GeoData::findPoint(glm::vec2 p) const
{
	return findPoint(p, *faces_begin());
}

GeoData::PointFace::PointFace(const glm::vec2 p, const GeoData * mesh) : PointFace {p, mesh, *mesh->faces_begin()} { }

GeoData::PointFace::PointFace(const glm::vec2 p, const GeoData * mesh, FaceHandle start) :
	PointFace {p, mesh->findPoint(p, start)}
{
}

GeoData::FaceHandle
GeoData::PointFace::face(const GeoData * mesh, FaceHandle start) const
{
	if (_face.is_valid()) {
		assert(mesh->triangleContainsPoint(point, _face));
		return _face;
	}
	else {
		return (_face = mesh->findPoint(point, start));
	}
}

GeoData::FaceHandle
GeoData::PointFace::face(const GeoData * mesh) const
{
	return face(mesh, *mesh->faces_begin());
}

namespace {
	template<template<typename> typename Op>
	[[nodiscard]] constexpr inline auto
	pointLineOp(const glm::vec2 p, const glm::vec2 e1, const glm::vec2 e2)
	{
		return Op {}((e2.x - e1.x) * (p.y - e1.y), (e2.y - e1.y) * (p.x - e1.x));
	}

	constexpr auto pointLeftOfLine = pointLineOp<std::greater>;
	constexpr auto pointLeftOfOrOnLine = pointLineOp<std::greater_equal>;

	static_assert(pointLeftOfLine({1, 2}, {1, 1}, {2, 2}));
	static_assert(pointLeftOfLine({2, 1}, {2, 2}, {1, 1}));
	static_assert(pointLeftOfLine({2, 2}, {1, 2}, {2, 1}));
	static_assert(pointLeftOfLine({1, 1}, {2, 1}, {1, 2}));

	[[nodiscard]] constexpr inline bool
	linesCross(const glm::vec2 a1, const glm::vec2 a2, const glm::vec2 b1, const glm::vec2 b2)
	{
		return pointLeftOfLine(a2, b1, b2) && pointLeftOfLine(a1, b2, b1) && pointLeftOfLine(b1, a1, a2)
				&& pointLeftOfLine(b2, a2, a1);
	}

	static_assert(linesCross({1, 1}, {2, 2}, {1, 2}, {2, 1}));
}

OpenMesh::FaceHandle
GeoData::findPoint(glm::vec2 p, OpenMesh::FaceHandle f) const
{
	while (f.is_valid() && !triangleContainsPoint(p, triangle<2>(f))) {
		for (auto next = cfh_iter(f); next.is_valid(); ++next) {
			f = opposite_face_handle(*next);
			if (f.is_valid()) {
				const auto e1 = point(to_vertex_handle(*next));
				const auto e2 = point(to_vertex_handle(opposite_halfedge_handle(*next)));
				if (pointLeftOfLine(p, e1, e2)) {
					break;
				}
			}
			f.reset();
		}
	}
	return f;
}

glm::vec3
GeoData::positionAt(const PointFace & p) const
{
	glm::vec3 out {};
	const auto t = triangle<3>(p.face(this));
	glm::intersectLineTriangle(p.point ^ 0.F, up, t[0], t[1], t[2], out);
	return p.point ^ out[0];
}

[[nodiscard]] std::optional<glm::vec3>
GeoData::intersectRay(const Ray & ray) const
{
	return intersectRay(ray, findPoint(ray.start));
}

[[nodiscard]] std::optional<glm::vec3>
GeoData::intersectRay(const Ray & ray, FaceHandle face) const
{
	std::optional<glm::vec3> out;
	walkUntil(PointFace {ray.start, face}, ray.start + (ray.direction * 10000.F), [&out, &ray, this](FaceHandle face) {
		glm::vec2 bari {};
		float dist {};
		const auto t = triangle<3>(face);
		if (glm::intersectRayTriangle(ray.start, ray.direction, t[0], t[1], t[2], bari, dist)) {
			out = t * bari;
			return true;
		}
		return false;
	});
	return out;
}

void
GeoData::walk(const PointFace & from, const glm::vec2 to, const std::function<void(FaceHandle)> & op) const
{
	walkUntil(from, to, [&op](const auto & fh) {
		op(fh);
		return false;
	});
}

void
GeoData::walkUntil(const PointFace & from, const glm::vec2 to, const std::function<bool(FaceHandle)> & op) const
{
	assert(from.face(this).is_valid()); // TODO replace with a boundary search
	auto f = from.face(this);
	FaceHandle previousFace;
	while (f.is_valid() && !op(f)) {
		for (auto next = cfh_iter(f); next.is_valid(); ++next) {
			f = opposite_face_handle(*next);
			if (f.is_valid() && f != previousFace) {
				const auto e1 = point(to_vertex_handle(*next));
				const auto e2 = point(to_vertex_handle(opposite_halfedge_handle(*next)));
				if (linesCross(from.point, to, e1, e2)) {
					previousFace = f;
					break;
				}
			}
			f.reset();
		}
	}
}

bool
GeoData::triangleContainsPoint(const glm::vec2 p, const Triangle<2> & t)
{
	return pointLeftOfOrOnLine(p, t[0], t[1]) && pointLeftOfOrOnLine(p, t[1], t[2])
			&& pointLeftOfOrOnLine(p, t[2], t[0]);
}

bool
GeoData::triangleContainsPoint(const glm::vec2 p, FaceHandle face) const
{
	return triangleContainsPoint(p, triangle<2>(face));
}
