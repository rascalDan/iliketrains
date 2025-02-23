#include "geoData.h"
#include "collections.h"
#include "geometricPlane.h"
#include "util.h"
#include <fstream>
#include <glm/gtx/intersect.hpp>
#include <maths.h>
#include <ranges>
#include <set>
#ifndef NDEBUG
#	include <stream_support.h>
#endif

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

	return mesh;
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
				const auto nextPoints = points(toVertexHandles(next));
				if (linesCrossLtR(from.point, to, nextPoints.second, nextPoints.first)) {
					step.exitHalfedge = next;
					step.exitPosition
							= linesIntersectAt(from.point.xy(), to.xy(), nextPoints.second.xy(), nextPoints.first.xy())
									  .value();
					break;
				}
			}
			step.current.reset();
		}
	}
}

void
GeoData::walk(const PointFace & from, GlobalPosition2D to, GlobalPosition2D centre, Consumer<WalkStepCurve> op) const
{
	walkUntil(from, to, centre, [&op](const auto & fh) {
		op(fh);
		return false;
	});
}

void
GeoData::walkUntil(const PointFace & from, GlobalPosition2D to, GlobalPosition2D centre, Tester<WalkStepCurve> op) const
{
	WalkStepCurve step {WalkStep {.current = from.face(this)}};
	if (!step.current.is_valid()) {
		const auto entryEdge = findEntry(from.point, to);
		if (!entryEdge.is_valid()) {
			return;
		}
		step.current = opposite_face_handle(entryEdge);
	}
	ArcSegment arc {centre, from.point, to};
	step.angle = arc.first;
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
					arc.first = std::nextafter(step.angle = intersect.value().second, INFINITY);
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

OpenMesh::VertexHandle
GeoData::setPoint(GlobalPosition3D tsPoint, const RelativeDistance nearNodeTolerance)
{
	const auto face = findPoint(tsPoint);
	const auto distFromTsPoint = vertexDistanceFunction<2>(tsPoint);
	// Check vertices
	if (const auto nearest = std::ranges::min(fv_range(face) | std::views::transform(distFromTsPoint), {}, GetSecond);
			nearest.second < nearNodeTolerance) {
		point(nearest.first).z = tsPoint.z;
		return nearest.first;
	}
	// Check edges
	if (const auto nearest = std::ranges::min(fh_range(face) | std::views::transform(distFromTsPoint), {}, GetSecond);
			nearest.second < nearNodeTolerance) {
		const auto from = point(from_vertex_handle(nearest.first)).xy();
		const auto to = point(to_vertex_handle(nearest.first)).xy();
		const auto v = vector_normal(from - to);
		const auto inter = linesIntersectAt(from, to, tsPoint.xy(), tsPoint.xy() + v);
		if (!inter) [[unlikely]] {
			throw std::runtime_error("Perpendicular lines do not cross");
		}
		return split_copy(edge_handle(nearest.first), *inter || tsPoint.z);
	}
	// Nothing close, split face
	return split_copy(face, tsPoint);
};

std::vector<GeoData::FaceHandle>
GeoData::setHeights(const std::span<const GlobalPosition3D> triangleStrip, const SetHeightsOpts & opts)
{
	if (triangleStrip.size() < 3) {
		return {};
	}
	const auto stripMinMax = std::ranges::minmax(triangleStrip, {}, &GlobalPosition3D::z);
	lowerExtent.z = std::min(upperExtent.z, stripMinMax.min.z);
	upperExtent.z = std::max(upperExtent.z, stripMinMax.max.z);

	class SetHeights {
	public:
		SetHeights(GeoData * geoData, const std::span<const GlobalPosition3D> triangleStrip) :
			geoData(geoData), triangleStrip {triangleStrip},
			strip {materializeRange(triangleStrip | triangleTriples | std::views::transform([](const auto & newVert) {
				return std::make_from_tuple<Triangle<3>>(newVert);
			}))}
		{
		}

		std::vector<VertexHandle>
		createVerticesForStrip(RelativeDistance nearNodeTolerance)
		{
			// New vertices for each vertex in triangleStrip
			const auto newVerts
					= materializeRange(triangleStrip | std::views::transform([this, nearNodeTolerance](auto v) {
						  return geoData->setPoint(v, nearNodeTolerance);
					  }));
			std::ranges::copy(newVerts, std::inserter(newOrChangedVerts, newOrChangedVerts.end()));
#ifndef NDEBUG
			geoData->sanityCheck();
#endif
			return newVerts;
		}

		const Triangle<3> *
		getTriangle(const GlobalPosition2D point) const
		{
			if (const auto t = std::ranges::find_if(strip,
						[point](const auto & triangle) {
							return triangle.containsPoint(point);
						});
					t != strip.end()) {
				return &*t;
			}
			return nullptr;
		}

		void
		doBoundaryPart(VertexHandle start, VertexHandle end, const Triangle<3> & triangle,
				const RelativeDistance nearNodeTolerance)
		{
			boundaryTriangles.emplace(start, &triangle);
			const auto endPoint = geoData->point(end);
			while (!std::ranges::contains(geoData->vv_range(start), end)) {
				const auto startPoint = geoData->point(start);
				const auto distanceToEndPoint = distance(startPoint.xy(), endPoint.xy());
				if (std::ranges::any_of(geoData->vv_range(start), [&](const auto & adjVertex) {
						const auto adjPoint = geoData->point(adjVertex);
						if (distance(adjPoint.xy(), endPoint.xy()) < distanceToEndPoint
								&& (Triangle<2> {startPoint, endPoint, adjPoint}.area()
										   / distance(startPoint.xy(), endPoint.xy()))
										< nearNodeTolerance) {
							start = adjVertex;
							newOrChangedVerts.emplace(start);
							boundaryTriangles.emplace(start, &triangle);
							geoData->point(start).z = triangle.positionOnPlane(adjPoint).z;
							return true;
						}
						return false;
					})) {
					continue;
				}
				if (std::ranges::any_of(geoData->voh_range(start), [&](const auto & outHalf) {
						const auto next = geoData->next_halfedge_handle(outHalf);
						const auto nexts
								= std::array {geoData->from_vertex_handle(next), geoData->to_vertex_handle(next)};
						const auto nextPoints = nexts | std::views::transform([this](const auto v) {
							return std::make_pair(v, geoData->point(v));
						});
						if (linesCross(startPoint, endPoint, nextPoints.front().second, nextPoints.back().second)) {
							if (const auto intersection = linesIntersectAt(startPoint.xy(), endPoint.xy(),
										nextPoints.front().second.xy(), nextPoints.back().second.xy())) {
								if (const auto nextEdge = geoData->shouldFlip(next, startPoint)) {
									geoData->flip(*nextEdge);
									return true;
								}
								start = geoData->split_copy(
										geoData->edge_handle(next), triangle.positionOnPlane(*intersection));
								newOrChangedVerts.emplace(start);
								boundaryTriangles.emplace(start, &triangle);
								return true;
							}
							throw std::runtime_error("Crossing lines don't intersect");
						}
						return false;
					})) {
					continue;
				}
#ifndef NDEBUG
				CLOG(start);
				CLOG(startPoint);
				CLOG(end);
				CLOG(endPoint);
				for (const auto v : geoData->vv_range(start)) {
					CLOG(geoData->point(v));
				}
				geoData->sanityCheck();
#endif
				throw std::runtime_error(
						std::format("Could not navigate to ({}, {}, {})", endPoint.x, endPoint.y, endPoint.z));
			}
		}

		void
		cutBoundary(const std::vector<VertexHandle> & newVerts, RelativeDistance nearNodeTolerance)
		{
			// Cut along each edge of triangleStrip AB, AC, BC, BD, CD, CE etc
			std::ranges::for_each(newVerts | std::views::adjacent<3>,
					[this, nearNodeTolerance, triangle = strip.begin()](const auto & verts) mutable {
						const auto & [a, _, c] = verts;
						doBoundaryPart(a, c, *triangle, nearNodeTolerance);
						triangle++;
					});
			doBoundaryPart(*++newVerts.begin(), newVerts.front(), strip.front(), nearNodeTolerance);
			doBoundaryPart(*++newVerts.rbegin(), newVerts.back(), strip.back(), nearNodeTolerance);
		}

		using HeightSetTodo = std::multimap<VertexHandle, VertexHandle>;

		HeightSetTodo
		setSurfaceHeights(const std::span<const VertexHandle> newVerts)
		{
			HeightSetTodo out;
			auto setSurfaceVertexHeight = [this, &out](const auto & setSurfaceVertexHeight, const VertexHandle vertex,
												  const VertexHandle previousVertex) -> void {
				if (surfaceVerts.contains(vertex)) {
					return;
				}
				auto & point = geoData->point(vertex);
				auto triangle = getTriangle(point);
				if (!triangle) {
					if (const auto boundaryVertex = boundaryTriangles.find(vertex);
							boundaryVertex != boundaryTriangles.end()) {
						triangle = boundaryVertex->second;
					}
				}
				if (triangle) { // point within the new strip, adjust vertically by triangle
					point.z = triangle->positionOnPlane(point).z;
					newOrChangedVerts.emplace(vertex);
					surfaceVerts.emplace(vertex);
					for (const auto nextVertex : geoData->vv_range(vertex)) {
						setSurfaceVertexHeight(setSurfaceVertexHeight, nextVertex, vertex);
					}
				}
				else if (previousVertex.is_valid()) {
					out.emplace(vertex, previousVertex);
				}
			};
			for (const auto vertex : newVerts) {
				setSurfaceVertexHeight(setSurfaceVertexHeight, vertex, VertexHandle {});
			}
			return out;
		}

		void
		setHalfedgeToHeight(HeightSetTodo & nexts, HeightSetTodo::const_iterator verticesBegin,
				HeightSetTodo::const_iterator verticesEnd, const RelativeDistance maxSlope)
		{
			const auto vertex = verticesBegin->first;
			auto & point = geoData->point(vertex);
			const auto minMaxHeight = std::accumulate(verticesBegin, verticesEnd,
					std::pair<GlobalDistance, GlobalDistance> {
							std::numeric_limits<GlobalDistance>::min(),
							std::numeric_limits<GlobalDistance>::max(),
					},
					[this, maxSlope, point](auto limit, auto previousVertexItr) {
						const auto & fromPoint = geoData->point(previousVertexItr.second);
						const auto maxOffset = static_cast<GlobalDistance>(
								std::round(maxSlope * ::distance<2>(fromPoint.xy(), point.xy())));
						limit.first = std::max(limit.first, fromPoint.z - maxOffset);
						limit.second = std::min(limit.second, fromPoint.z + maxOffset);
						return limit;
					});

			const auto newHeight = std::clamp(point.z, minMaxHeight.first, minMaxHeight.second);
			if (newHeight != point.z) {
				point.z = newHeight;
				newOrChangedVerts.emplace(vertex);
				for (const auto nextVertex : geoData->vv_range(vertex)) {
					if (!std::ranges::contains(verticesBegin, verticesEnd, nextVertex, GetSecond)
							&& !boundaryTriangles.contains(nextVertex)) {
						nexts.emplace(nextVertex, vertex);
					}
				}
			}
		}

		void
		setHeightsAsRequired(HeightSetTodo starts, RelativeDistance maxSlope)
		{
			while (!starts.empty()) {
				HeightSetTodo nexts;

				for (const auto chunk : starts | std::views::chunk_by([](const auto a, const auto b) {
						 return a.first == b.first;
					 })) {
					setHalfedgeToHeight(nexts, chunk.begin(), chunk.end(), maxSlope);
				}
				starts = std::move(nexts);
			}
		}

		std::vector<FaceHandle>
		setSurface(const Surface * surface)
		{
			std::vector<FaceHandle> out;
			auto surfaceStripWalk = [this, surface, &out](const auto & surfaceStripWalk, const auto & face) -> void {
				if (!geoData->property(geoData->surface, face)) {
					geoData->property(geoData->surface, face) = surface;
					out.emplace_back(face);
					std::ranges::for_each(
							geoData->ff_range(face), [this, &surfaceStripWalk](const auto & adjacentFaceHandle) {
								if (getTriangle(geoData->triangle<2>(adjacentFaceHandle).centroid())) {
									surfaceStripWalk(surfaceStripWalk, adjacentFaceHandle);
								}
							});
				}
			};
			for (const auto & triangle : strip) {
				surfaceStripWalk(surfaceStripWalk, geoData->findPoint(triangle.centroid()));
			}
			return out;
		}

		std::vector<GeoData::FaceHandle>
		run(const SetHeightsOpts & opts)
		{
			const std::vector<VertexHandle> newVerts = createVerticesForStrip(opts.nearNodeTolerance);
			cutBoundary(newVerts, opts.nearNodeTolerance);
			setHeightsAsRequired(setSurfaceHeights(newVerts), opts.maxSlope);
			const auto out = setSurface(opts.surface);

			geoData->expandVerts(newOrChangedVerts);
			geoData->updateAllVertexNormals(newOrChangedVerts);
			geoData->afterChange();

			return out;
		}

	private:
		GeoData * geoData;
		const std::span<const GlobalPosition3D> triangleStrip;
		const std::vector<Triangle<3>> strip;
		std::set<VertexHandle> newOrChangedVerts;
		std::set<VertexHandle> surfaceVerts;
		std::map<VertexHandle, const Triangle<3> *> boundaryTriangles;
	};

	return SetHeights {this, triangleStrip}.run(opts);
}

void
GeoData::afterChange()
{
}
