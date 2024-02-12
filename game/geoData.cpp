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
	xllcorner *= 1000;
	yllcorner *= 1000;
	cellsize *= 1000;
	std::vector<VertexHandle> vertices;
	vertices.reserve(ncols * nrows);
	GeoData mesh;
	mesh.lowerExtent = {xllcorner, yllcorner, std::numeric_limits<GlobalDistance>::max()};
	mesh.upperExtent = {xllcorner + (cellsize * (ncols - 1)), yllcorner + (cellsize * (nrows - 1)),
			std::numeric_limits<GlobalDistance>::min()};
	for (size_t row = 0; row < nrows; ++row) {
		for (size_t col = 0; col < ncols; ++col) {
			float heightf = 0;
			f >> heightf;
			const auto height = static_cast<GlobalDistance>(std::round(heightf * 1000.F));
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
	mesh.update_vertex_normals_only();

	return mesh;
};

template<typename T> constexpr static T GRID_SIZE = 10'000;

GeoData
GeoData::createFlat(GlobalPosition2D lower, GlobalPosition2D upper, GlobalDistance h)
{
	GeoData mesh;

	mesh.lowerExtent = {lower, h};
	mesh.upperExtent = {upper, h};

	std::vector<VertexHandle> vertices;
	for (GlobalDistance row = lower.x; row < upper.x; row += GRID_SIZE<GlobalDistance>) {
		for (GlobalDistance col = lower.y; col < upper.y; col += GRID_SIZE<GlobalDistance>) {
			vertices.push_back(mesh.add_vertex({col, row, h}));
		}
	}

	const auto nrows = static_cast<size_t>(std::ceil(float(upper.x - lower.x) / GRID_SIZE<RelativeDistance>));
	const auto ncols = static_cast<size_t>(std::ceil(float(upper.y - lower.y) / GRID_SIZE<RelativeDistance>));
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

	mesh.update_vertex_normals_only();

	return mesh;
}

OpenMesh::FaceHandle
GeoData::findPoint(GlobalPosition2D p) const
{
	return findPoint(p, *faces_begin());
}

GeoData::PointFace::PointFace(const GlobalPosition2D p, const GeoData * mesh) :
	PointFace {p, mesh, *mesh->faces_begin()}
{
}

GeoData::PointFace::PointFace(const GlobalPosition2D p, const GeoData * mesh, FaceHandle start) :
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
	pointLineOp(const GlobalPosition2D p, const GlobalPosition2D e1, const GlobalPosition2D e2)
	{
		return Op {}(CalcDistance(e2.x - e1.x) * CalcDistance(p.y - e1.y),
				CalcDistance(e2.y - e1.y) * CalcDistance(p.x - e1.x));
	}

	constexpr auto pointLeftOfLine = pointLineOp<std::greater>;
	constexpr auto pointLeftOfOrOnLine = pointLineOp<std::greater_equal>;

	static_assert(pointLeftOfLine({1, 2}, {1, 1}, {2, 2}));
	static_assert(pointLeftOfLine({2, 1}, {2, 2}, {1, 1}));
	static_assert(pointLeftOfLine({2, 2}, {1, 2}, {2, 1}));
	static_assert(pointLeftOfLine({1, 1}, {2, 1}, {1, 2}));
	static_assert(pointLeftOfOrOnLine({310000000, 490000000}, {310000000, 490000000}, {310050000, 490050000}));
	static_assert(pointLeftOfOrOnLine({310000000, 490000000}, {310050000, 490050000}, {310000000, 490050000}));
	static_assert(pointLeftOfOrOnLine({310000000, 490000000}, {310000000, 490050000}, {310000000, 490000000}));

	[[nodiscard]] constexpr inline bool
	linesCross(
			const GlobalPosition2D a1, const GlobalPosition2D a2, const GlobalPosition2D b1, const GlobalPosition2D b2)
	{
		return (pointLeftOfLine(a2, b1, b2) == pointLeftOfLine(a1, b2, b1))
				&& (pointLeftOfLine(b1, a1, a2) == pointLeftOfLine(b2, a2, a1));
	}

	static_assert(linesCross({1, 1}, {2, 2}, {1, 2}, {2, 1}));
	static_assert(linesCross({2, 2}, {1, 1}, {1, 2}, {2, 1}));

	[[nodiscard]] constexpr inline bool
	linesCrossLtR(
			const GlobalPosition2D a1, const GlobalPosition2D a2, const GlobalPosition2D b1, const GlobalPosition2D b2)
	{
		return pointLeftOfLine(a2, b1, b2) && pointLeftOfLine(a1, b2, b1) && pointLeftOfLine(b1, a1, a2)
				&& pointLeftOfLine(b2, a2, a1);
	}

	static_assert(linesCrossLtR({1, 1}, {2, 2}, {1, 2}, {2, 1}));
	static_assert(!linesCrossLtR({2, 2}, {1, 1}, {1, 2}, {2, 1}));

	constexpr GlobalPosition3D
	positionOnTriangle(const GlobalPosition2D point, const GeoData::Triangle<3> & t)
	{
		const CalcPosition3D a = t[1] - t[0], b = t[2] - t[0];
		const auto n = crossProduct(a, b);
		return {point, ((n.x * t[0].x) + (n.y * t[0].y) + (n.z * t[0].z) - (n.x * point.x) - (n.y * point.y)) / n.z};
	}

	static_assert(positionOnTriangle({7, -2}, {{1, 2, 3}, {1, 0, 1}, {-2, 1, 0}}) == GlobalPosition3D {7, -2, 3});
}

OpenMesh::FaceHandle
GeoData::findPoint(GlobalPosition2D p, OpenMesh::FaceHandle f) const
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

GlobalPosition3D
GeoData::positionAt(const PointFace & p) const
{
	return positionOnTriangle(p.point, triangle<3>(p.face(this)));
}

[[nodiscard]] std::optional<GlobalPosition3D>
GeoData::intersectRay(const Ray<GlobalPosition3D> & ray) const
{
	return intersectRay(ray, findPoint(ray.start));
}

[[nodiscard]] std::optional<GlobalPosition3D>
GeoData::intersectRay(const Ray<GlobalPosition3D> & ray, FaceHandle face) const
{
	std::optional<GlobalPosition3D> out;
	walkUntil(PointFace {ray.start, face},
			ray.start.xy()
					+ GlobalPosition2D(ray.direction.xy() * RelativePosition2D(upperExtent.xy() - lowerExtent.xy())),
			[&out, &ray, this](FaceHandle face) {
				BaryPosition bari {};
				RelativeDistance dist {};
				const auto t = triangle<3>(face);
				if (ray.intersectTriangle(t.x, t.y, t.z, bari, dist)) {
					out = t * bari;
					return true;
				}
				return false;
			});
	return out;
}

void
GeoData::walk(const PointFace & from, const GlobalPosition2D to, const std::function<void(FaceHandle)> & op) const
{
	walkUntil(from, to, [&op](const auto & fh) {
		op(fh);
		return false;
	});
}

void
GeoData::walkUntil(const PointFace & from, const GlobalPosition2D to, const std::function<bool(FaceHandle)> & op) const
{
	auto f = from.face(this);
	if (!f.is_valid()) {
		const auto entryEdge = findEntry(from.point, to);
		if (!entryEdge.is_valid()) {
			return;
		}
		f = opposite_face_handle(entryEdge);
	}
	FaceHandle previousFace;
	while (f.is_valid() && !op(f)) {
		for (auto next = cfh_iter(f); next.is_valid(); ++next) {
			f = opposite_face_handle(*next);
			if (f.is_valid() && f != previousFace) {
				const auto e1 = point(to_vertex_handle(*next));
				const auto e2 = point(to_vertex_handle(opposite_halfedge_handle(*next)));
				if (linesCrossLtR(from.point, to, e1, e2)) {
					previousFace = f;
					break;
				}
			}
			f.reset();
		}
	}
}

void
GeoData::boundaryWalk(const std::function<void(HalfedgeHandle)> & op) const
{
	boundaryWalk(op, findBoundaryStart());
}

void
GeoData::boundaryWalk(const std::function<void(HalfedgeHandle)> & op, HalfedgeHandle start) const
{
	assert(is_boundary(start));
	boundaryWalkUntil(
			[&op](auto heh) {
				op(heh);
				return false;
			},
			start);
}

void
GeoData::boundaryWalkUntil(const std::function<bool(HalfedgeHandle)> & op) const
{
	boundaryWalkUntil(op, findBoundaryStart());
}

void
GeoData::boundaryWalkUntil(const std::function<bool(HalfedgeHandle)> & op, HalfedgeHandle start) const
{
	assert(is_boundary(start));
	if (!op(start)) {
		for (auto heh = next_halfedge_handle(start); heh != start; heh = next_halfedge_handle(heh)) {
			if (op(heh)) {
				break;
			}
		}
	}
}

GeoData::HalfedgeHandle
GeoData::findEntry(const GlobalPosition2D from, const GlobalPosition2D to) const
{
	HalfedgeHandle entry;
	boundaryWalkUntil([this, from, to, &entry](auto he) {
		const auto e1 = point(to_vertex_handle(he));
		const auto e2 = point(to_vertex_handle(opposite_halfedge_handle(he)));
		if (linesCrossLtR(from, to, e1, e2)) {
			entry = he;
			return true;
		}
		return false;
	});
	return entry;
}

bool
GeoData::triangleContainsPoint(const GlobalPosition2D p, const Triangle<2> & t)
{
	return pointLeftOfOrOnLine(p, t[0], t[1]) && pointLeftOfOrOnLine(p, t[1], t[2])
			&& pointLeftOfOrOnLine(p, t[2], t[0]);
}

bool
GeoData::triangleContainsPoint(const GlobalPosition2D p, FaceHandle face) const
{
	return triangleContainsPoint(p, triangle<2>(face));
}

GeoData::HalfedgeHandle
GeoData::findBoundaryStart() const
{
	return *std::find_if(halfedges_begin(), halfedges_end(), [this](const auto heh) {
		return is_boundary(heh);
	});
}

void
GeoData::update_vertex_normals_only()
{
	for (auto vh : all_vertices()) {
		Normal3D n;
		calc_vertex_normal_correct(vh, n);
		this->set_normal(vh, glm::normalize(n));
	}
}
