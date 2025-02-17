#include "geoDataMesh.h"
#include <format>
#ifndef NDEBUG
#	include <stream_support.h>
#endif

OpenMesh::FaceHandle
GeoDataMesh::findPoint(GlobalPosition2D coord) const
{
	return findPoint(coord, *faces_sbegin());
}

GeoDataMesh::PointFace::PointFace(const GlobalPosition2D coord, const GeoDataMesh * mesh) :
	PointFace {coord, mesh, *mesh->faces_sbegin()}
{
}

GeoDataMesh::PointFace::PointFace(const GlobalPosition2D coord, const GeoDataMesh * mesh, FaceHandle start) :
	PointFace {coord, mesh->findPoint(coord, start)}
{
}

OpenMesh::FaceHandle
GeoDataMesh::PointFace::face(const GeoDataMesh * mesh, FaceHandle start) const
{
	if (faceCache.is_valid() && mesh->faceContainsPoint(point, faceCache)) {
		return faceCache;
	}
	return (faceCache = mesh->findPoint(point, start));
}

OpenMesh::FaceHandle
GeoDataMesh::PointFace::face(const GeoDataMesh * mesh) const
{
	return face(mesh, *mesh->faces_sbegin());
}

GeoDataMesh::HalfEdgeVertices
GeoDataMesh::toVertexHandles(HalfedgeHandle halfEdge) const
{
	return {from_vertex_handle(halfEdge), to_vertex_handle(halfEdge)};
}

GeoDataMesh::HalfEdgePoints
GeoDataMesh::points(HalfEdgeVertices vertices) const
{
	return {point(vertices.first), point(vertices.second)};
}

OpenMesh::FaceHandle
GeoDataMesh::findPoint(const GlobalPosition2D coord, OpenMesh::FaceHandle face) const
{
	while (face.is_valid() && !triangle<2>(face).containsPoint(coord)) {
		for (auto next = cfh_iter(face); next.is_valid(); ++next) {
			face = opposite_face_handle(*next);
			if (face.is_valid()) {
				const auto nextPoints = points(toVertexHandles(*next));
				if (pointLeftOfLine(coord, nextPoints.second, nextPoints.first)) {
					break;
				}
			}
			face.reset();
		}
	}
	return face;
}

GlobalPosition3D
GeoDataMesh::positionAt(const PointFace & coord) const
{
	return triangle<3>(coord.face(this)).positionOnPlane(coord.point);
}

bool
GeoDataMesh::faceContainsPoint(const GlobalPosition2D coord, FaceHandle face) const
{
	return triangle<2>(face).containsPoint(coord);
}

OpenMesh::HalfedgeHandle
GeoDataMesh::findBoundaryStart() const
{
	return *std::find_if(halfedges_sbegin(), halfedges_end(), [this](const auto heh) {
		return is_boundary(heh);
	});
}

[[nodiscard]] RelativePosition3D
GeoDataMesh::difference(const HalfedgeHandle heh) const
{
	return ::difference(point(to_vertex_handle(heh)), point(from_vertex_handle(heh)));
}

[[nodiscard]] GlobalPosition3D
GeoDataMesh::centre(const HalfedgeHandle heh) const
{
	const auto hehPoints = points(toVertexHandles(heh));
	return midpoint(hehPoints.first, hehPoints.second);
}

#ifndef NDEBUG
void
GeoDataMesh::sanityCheck(const std::source_location & loc) const
{
	if (const auto upSideDown = std::ranges::count_if(faces(), [this](const auto face) {
			if (!triangle<2>(face).isUp()) {
				for (const auto vertex : fv_range(face)) {
					CLOG(point(vertex));
				}
				return true;
			}
			return false;
		}) > 0) {
		throw std::logic_error(std::format(
				"{} upside down faces detected - checked from {}:{}", upSideDown, loc.function_name(), loc.line()));
	}
}
#endif

bool
GeoDataMesh::canFlip(const HalfedgeHandle edge) const
{
	const auto opposite = opposite_halfedge_handle(edge);
	const auto pointA = point(to_vertex_handle(edge));
	const auto pointB = point(to_vertex_handle(opposite));
	const auto pointC = point(to_vertex_handle(next_halfedge_handle(edge)));
	const auto pointD = point(to_vertex_handle(next_halfedge_handle(opposite)));

	return Triangle<2> {pointC, pointB, pointD}.isUp() && Triangle<2> {pointA, pointC, pointD}.isUp();
};

std::optional<OpenMesh::EdgeHandle>
GeoDataMesh::shouldFlip(const HalfedgeHandle next, const GlobalPosition2D startPoint) const
{
	if (const auto nextEdge = edge_handle(next); is_flip_ok(nextEdge) && canFlip(next)) {
		const auto oppositePoint = point(to_vertex_handle(next_halfedge_handle(opposite_halfedge_handle(next)))).xy();
		if (distance<2>(startPoint, oppositePoint) < length<2>(next)) {
			return nextEdge;
		}
	}
	return std::nullopt;
};
