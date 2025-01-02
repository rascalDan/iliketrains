#include "geoData.h"
#include "collections.h"
#include "geometricPlane.h"
#include <fstream>
#include <glm/gtx/intersect.hpp>
#include <maths.h>
#include <ranges>
#include <set>

GeoData::GeoData()
{
	add_property(surface);
}

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
	mesh.generation++;
	mesh.updateAllVertexNormals();

	return mesh;
};

constexpr static GlobalDistance GRID_SIZE = 10'000;

GeoData
GeoData::createFlat(GlobalPosition2D lower, GlobalPosition2D upper, GlobalDistance h)
{
	assert((upper - lower) % GRID_SIZE == GlobalPosition2D {});
	GeoData mesh;

	mesh.lowerExtent = {lower, h};
	mesh.upperExtent = {upper, h};

	std::vector<VertexHandle> vertices;
	for (GlobalDistance row = lower.x; row <= upper.x; row += GRID_SIZE) {
		for (GlobalDistance col = lower.y; col <= upper.y; col += GRID_SIZE) {
			vertices.push_back(mesh.add_vertex({col, row, h}));
		}
	}

	const auto n = glm::vec<2, size_t> {((upper - lower) / GRID_SIZE) + 1};
	for (auto row = 1U; row < n.x; ++row) {
		for (auto col = 1U; col < n.y; ++col) {
			mesh.add_face({
					vertices[n.y * (row - 1) + (col - 1)],
					vertices[n.y * (row - 0) + (col - 0)],
					vertices[n.y * (row - 0) + (col - 1)],
			});
			mesh.add_face({
					vertices[n.y * (row - 1) + (col - 1)],
					vertices[n.y * (row - 1) + (col - 0)],
					vertices[n.y * (row - 0) + (col - 0)],
			});
		}
	}

	mesh.updateAllVertexNormals();
	mesh.generation++;

	return mesh;
}

OpenMesh::FaceHandle
GeoData::findPoint(GlobalPosition2D p) const
{
	return findPoint(p, *faces_sbegin());
}

GeoData::PointFace::PointFace(const GlobalPosition2D p, const GeoData * mesh) :
	PointFace {p, mesh, *mesh->faces_sbegin()}
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
	return face(mesh, *mesh->faces_sbegin());
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

[[nodiscard]] GeoData::IntersectionResult
GeoData::intersectRay(const Ray<GlobalPosition3D> & ray) const
{
	return intersectRay(ray, findPoint(ray.start));
}

[[nodiscard]] GeoData::IntersectionResult
GeoData::intersectRay(const Ray<GlobalPosition3D> & ray, FaceHandle face) const
{
	GeoData::IntersectionResult out;
	walkUntil(PointFace {ray.start, face},
			ray.start.xy() + (ray.direction.xy() * RelativePosition2D(upperExtent.xy() - lowerExtent.xy())),
			[&out, &ray, this](const auto & step) {
				BaryPosition bari {};
				RelativeDistance dist {};
				const auto t = triangle<3>(step.current);
				if (ray.intersectTriangle(t.x, t.y, t.z, bari, dist)) {
					out.emplace(t * bari, step.current);
					return true;
				}
				return false;
			});
	return out;
}

void
GeoData::walk(const PointFace & from, const GlobalPosition2D to, Consumer<WalkStep> op) const
{
	walkUntil(from, to, [&op](const auto & fh) {
		op(fh);
		return false;
	});
}

void
GeoData::walkUntil(const PointFace & from, const GlobalPosition2D to, Tester<WalkStep> op) const
{
	WalkStep step {
			.current = from.face(this),
	};
	if (!step.current.is_valid()) {
		const auto entryEdge = findEntry(from.point, to);
		if (!entryEdge.is_valid()) {
			return;
		}
		step.current = opposite_face_handle(entryEdge);
	}
	while (step.current.is_valid() && !op(step)) {
		step.previous = step.current;
		for (const auto next : fh_range(step.current)) {
			step.current = opposite_face_handle(next);
			if (step.current.is_valid() && step.current != step.previous) {
				const auto e1 = point(to_vertex_handle(next));
				const auto e2 = point(to_vertex_handle(opposite_halfedge_handle(next)));
				if (linesCrossLtR(from.point, to, e1, e2)) {
					step.exitHalfedge = next;
					step.exitPosition = linesIntersectAt(from.point.xy(), to.xy(), e1.xy(), e2.xy()).value();
					break;
				}
			}
			step.current.reset();
		}
	}
}

void
GeoData::walk(const PointFace & from, GlobalPosition2D to, GlobalPosition2D centre, Consumer<WalkStep> op) const
{
	walkUntil(from, to, centre, [&op](const auto & fh) {
		op(fh);
		return false;
	});
}

void
GeoData::walkUntil(const PointFace & from, GlobalPosition2D to, GlobalPosition2D centre, Tester<WalkStep> op) const
{
	WalkStep step {
			.current = from.face(this),
	};
	if (!step.current.is_valid()) {
		const auto entryEdge = findEntry(from.point, to);
		if (!entryEdge.is_valid()) {
			return;
		}
		step.current = opposite_face_handle(entryEdge);
	}
	ArcSegment arc {centre, from.point, to};
	while (step.current.is_valid() && !op(step)) {
		step.previous = step.current;
		for (const auto next : fh_range(step.current)) {
			step.current = opposite_face_handle(next);
			if (step.current.is_valid()) {
				const auto e1 = point(to_vertex_handle(next));
				const auto e2 = point(to_vertex_handle(opposite_halfedge_handle(next)));
				if (const auto intersect = arc.crossesLineAt(e1, e2)) {
					step.exitHalfedge = next;
					arc.ep0 = step.exitPosition = intersect.value().first;
					arc.first = std::nextafter(intersect.value().second, INFINITY);
					break;
				}
			}
			step.current.reset();
		}
	}
}

void
GeoData::boundaryWalk(Consumer<HalfedgeHandle> op) const
{
	boundaryWalk(op, findBoundaryStart());
}

void
GeoData::boundaryWalk(Consumer<HalfedgeHandle> op, HalfedgeHandle start) const
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
GeoData::boundaryWalkUntil(Tester<HalfedgeHandle> op) const
{
	boundaryWalkUntil(op, findBoundaryStart());
}

void
GeoData::boundaryWalkUntil(Tester<HalfedgeHandle> op, HalfedgeHandle start) const
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
	return *std::find_if(halfedges_sbegin(), halfedges_end(), [this](const auto heh) {
		return is_boundary(heh);
	});
}

[[nodiscard]] RelativePosition3D
GeoData::difference(const HalfedgeHandle heh) const
{
	return ::difference(point(to_vertex_handle(heh)), point(from_vertex_handle(heh)));
}

[[nodiscard]] RelativeDistance
GeoData::length(const HalfedgeHandle heh) const
{
	return glm::length(difference(heh));
}

[[nodiscard]] GlobalPosition3D
GeoData::centre(const HalfedgeHandle heh) const
{
	return point(from_vertex_handle(heh)) + (difference(heh) / 2.F);
}

void
GeoData::updateAllVertexNormals()
{
	updateAllVertexNormals(vertices());
}

template<std::ranges::range R>
void
GeoData::updateAllVertexNormals(const R & range)
{
	std::ranges::for_each(range, [this](const auto vertex) {
		updateVertexNormal(vertex);
	});
}

void
GeoData::updateVertexNormal(VertexHandle vertex)
{
	Normal3D n;
	calc_vertex_normal_correct(vertex, n);
	set_normal(vertex, glm::normalize(n));
}

bool
GeoData::triangleOverlapsTriangle(const Triangle<2> & a, const Triangle<2> & b)
{
	return triangleContainsPoint(a.x, b) || triangleContainsPoint(a.y, b) || triangleContainsPoint(a.z, b)
			|| triangleContainsPoint(b.x, a) || triangleContainsPoint(b.y, a) || triangleContainsPoint(b.z, a)
			|| linesCross(a.x, a.y, b.x, b.y) || linesCross(a.x, a.y, b.y, b.z) || linesCross(a.x, a.y, b.z, b.x)
			|| linesCross(a.y, a.z, b.x, b.y) || linesCross(a.y, a.z, b.y, b.z) || linesCross(a.y, a.z, b.z, b.x)
			|| linesCross(a.z, a.x, b.x, b.y) || linesCross(a.z, a.x, b.y, b.z) || linesCross(a.z, a.x, b.z, b.x);
}

bool
GeoData::triangleContainsTriangle(const Triangle<2> & a, const Triangle<2> & b)
{
	return triangleContainsPoint(a.x, b) && triangleContainsPoint(a.y, b) && triangleContainsPoint(a.z, b);
}

void
GeoData::setHeights(const std::span<const GlobalPosition3D> triangleStrip, const SetHeightsOpts & opts)
{
	if (triangleStrip.size() < 3) {
		return;
	}
	const auto stripMinMax = std::ranges::minmax(triangleStrip, {}, &GlobalPosition3D::z);
	lowerExtent.z = std::min(upperExtent.z, stripMinMax.min.z);
	upperExtent.z = std::max(upperExtent.z, stripMinMax.max.z);

	const auto vertexDistFrom = [this](GlobalPosition2D p) {
		return [p, this](const VertexHandle v) {
			return std::make_pair(v, glm::length(::difference(p, this->point(v).xy())));
		};
	};
	const auto vertexDistFromE = [this](GlobalPosition2D p) {
		return [p, this](const HalfedgeHandle e) {
			const auto fromPoint = point(from_vertex_handle(e)).xy();
			const auto toPoint = point(to_vertex_handle(e)).xy();
			return std::make_pair(e, Triangle<2> {fromPoint, toPoint, p}.height());
		};
	};

	std::set<VertexHandle> newOrChangedVerts;
	auto addVertexForNormalUpdate = [this, &newOrChangedVerts](const VertexHandle vertex) {
		newOrChangedVerts.emplace(vertex);
		std::ranges::copy(vv_range(vertex), std::inserter(newOrChangedVerts, newOrChangedVerts.end()));
	};

	auto newVertexOnFace = [this, &vertexDistFrom, &opts, &vertexDistFromE](GlobalPosition3D tsPoint) {
		const auto face = findPoint(tsPoint);
		// Check vertices
		if (const auto nearest = std::ranges::min(
					std::views::iota(fv_begin(face), fv_end(face)) | std::views::transform(vertexDistFrom(tsPoint)), {},
					&std::pair<VertexHandle, float>::second);
				nearest.second < opts.nearNodeTolerance) {
			point(nearest.first).z = tsPoint.z;
			return nearest.first;
		}
		// Check edges
		if (const auto nearest = std::ranges::min(
					std::views::iota(fh_begin(face), fh_end(face)) | std::views::transform(vertexDistFromE(tsPoint)),
					{}, &std::pair<HalfedgeHandle, float>::second);
				nearest.second < opts.nearNodeTolerance) {
			const auto from = point(from_vertex_handle(nearest.first)).xy();
			const auto to = point(to_vertex_handle(nearest.first)).xy();
			const auto v = vector_normal(from - to);
			const auto inter = linesIntersectAt(from, to, tsPoint.xy(), tsPoint.xy() + v);
			if (!inter) {
				throw std::runtime_error("Perpendicular lines do not cross");
			}
			return split_copy(edge_handle(nearest.first), *inter || tsPoint.z);
		}
		// Nothing close, split face
		return split_copy(face, tsPoint);
	};

	// New vertices for each vertex in triangleStrip
	std::vector<VertexHandle> newVerts;
	newVerts.reserve(triangleStrip.size());
	std::transform(triangleStrip.begin(), triangleStrip.end(), std::back_inserter(newVerts), newVertexOnFace);
	std::ranges::for_each(newVerts, addVertexForNormalUpdate);

	// Create temporary triangles from triangleStrip
	std::vector<Triangle<3>> strip;
	std::transform(
			strip_begin(triangleStrip), strip_end(triangleStrip), std::back_inserter(strip), [](const auto & newVert) {
				const auto [a, b, c] = newVert;
				return Triangle<3> {a, b, c};
			});
	auto getTriangle = [&strip](const auto point) -> const Triangle<3> * {
		if (const auto t = std::ranges::find_if(strip,
					[point](const auto & triangle) {
						return triangleContainsPoint(point, triangle);
					});
				t != strip.end()) {
			return &*t;
		}
		return nullptr;
	};

	// Cut along each edge of triangleStrip AB, AC, BC, BD, CD, CE etc
	std::map<VertexHandle, const Triangle<3> *> boundaryTriangles;
	auto doBoundaryPart = [this, &boundaryTriangles, &vertexDistFrom, &opts, &addVertexForNormalUpdate](
								  VertexHandle start, VertexHandle end, const Triangle<3> & triangle) {
		boundaryTriangles.emplace(start, &triangle);
		const auto endPoint = point(end);
		while (!std::ranges::contains(vv_range(start), end)) {
			const auto startPoint = point(start);
			if (std::ranges::none_of(voh_range(start), [&](const auto & outHalf) {
					const auto next = next_halfedge_handle(outHalf);
					const auto nexts = std::array {from_vertex_handle(next), to_vertex_handle(next)};
					const auto nextPoints = nexts | std::views::transform([this](const auto v) {
						return std::make_pair(v, this->point(v));
					});
					if (linesCross(startPoint, endPoint, nextPoints.front().second, nextPoints.back().second)) {
						if (const auto intersection = linesIntersectAt(startPoint.xy(), endPoint.xy(),
									nextPoints.front().second.xy(), nextPoints.back().second.xy())) {
							if (const auto nextDist
									= std::ranges::min(nexts | std::views::transform(vertexDistFrom(*intersection)), {},
											&std::pair<VertexHandle, float>::second);
									nextDist.second < opts.nearNodeTolerance) {
								point(nextDist.first).z = positionOnTriangle(point(nextDist.first), triangle).z;
								start = nextDist.first;
								return true;
							}
							else {
								start = split_copy(edge_handle(next), positionOnTriangle(*intersection, triangle));
							}
							addVertexForNormalUpdate(start);
							boundaryTriangles.emplace(start, &triangle);
							return true;
						}
						throw std::runtime_error("Crossing lines don't intersect");
					}
					return false;
				})) {
				throw std::runtime_error(
						std::format("Could not navigate to ({}, {}, {})", endPoint.x, endPoint.y, endPoint.z));
			}
		}
	};
	auto doBoundary = [&doBoundaryPart, triangle = strip.begin()](const auto & verts) mutable {
		const auto & [a, _, c] = verts;
		doBoundaryPart(a, c, *triangle);
		triangle++;
	};
	std::ranges::for_each(newVerts | std::views::adjacent<3>, doBoundary);
	doBoundaryPart(*++newVerts.begin(), newVerts.front(), strip.front());
	doBoundaryPart(*++newVerts.rbegin(), newVerts.back(), strip.back());

	std::set<HalfedgeHandle> done;
	std::set<HalfedgeHandle> todo;
	auto todoOutHalfEdges = [&todo, &done, this](const VertexHandle v) {
		std::copy_if(voh_begin(v), voh_end(v), std::inserter(todo, todo.end()), [&done](const auto & h) {
			return !done.contains(h);
		});
	};
	std::ranges::for_each(newVerts, todoOutHalfEdges);
	while (!todo.empty()) {
		const auto heh = todo.extract(todo.begin()).value();
		const auto fromVertex = from_vertex_handle(heh);
		const auto toVertex = to_vertex_handle(heh);
		const auto & fromPoint = point(fromVertex);
		auto & toPoint = point(toVertex);
		auto toTriangle = getTriangle(toPoint);
		if (!toTriangle) {
			if (const auto boundaryVertex = boundaryTriangles.find(toVertex);
					boundaryVertex != boundaryTriangles.end()) {
				toTriangle = boundaryVertex->second;
			}
		}
		if (toTriangle) { // point within the new strip, adjust vertically by triangle
			toPoint.z = positionOnTriangle(toPoint, *toTriangle).z;
			addVertexForNormalUpdate(toVertex);
			todoOutHalfEdges(toVertex);
		}
		else if (!toTriangle) { // point without the new strip, adjust vertically by limit
			const auto maxOffset = static_cast<GlobalDistance>(opts.maxSlope * glm::length(difference(heh).xy()));
			const auto newHeight = std::clamp(toPoint.z, fromPoint.z - maxOffset, fromPoint.z + maxOffset);
			if (newHeight != toPoint.z) {
				toPoint.z = newHeight;
				addVertexForNormalUpdate(toVertex);
				std::copy_if(voh_begin(toVertex), voh_end(toVertex), std::inserter(todo, todo.end()),
						[this, &boundaryTriangles](const auto & heh) {
							return !boundaryTriangles.contains(to_vertex_handle(heh));
						});
			}
		}
		done.insert(heh);
	}

	auto surfaceStripWalk = [this, &getTriangle, &opts](const auto & surfaceStripWalk, const auto & face) -> void {
		if (!property(surface, face)) {
			property(surface, face) = opts.surface;
			std::ranges::for_each(
					ff_range(face), [this, &getTriangle, &surfaceStripWalk](const auto & adjacentFaceHandle) {
						if (getTriangle(this->triangle<2>(adjacentFaceHandle).centroid())) {
							surfaceStripWalk(surfaceStripWalk, adjacentFaceHandle);
						}
					});
		}
	};
	for (const auto & triangle : strip) {
		surfaceStripWalk(surfaceStripWalk, findPoint(triangle.centroid()));
	}

	updateAllVertexNormals(newOrChangedVerts);
	generation++;
}

size_t
GeoData::getGeneration() const
{
	return generation;
}

void
GeoData::sanityCheck() const
{
	if (!std::ranges::all_of(faces(), [this](const auto face) {
			return triangle<2>(face).isUp();
		})) {
		throw std::logic_error("Upside down faces detected");
	}
}
