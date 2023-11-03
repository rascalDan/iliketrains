#include "terrain2.h"
#include <fstream>
#include <glm/gtx/intersect.hpp>
#include <maths.h>

TerrainMesh::TerrainMesh(const std::filesystem::path & input)
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
	for (size_t row = 0; row < nrows; ++row) {
		for (size_t col = 0; col < ncols; ++col) {
			float height = 0;
			f >> height;
			vertices.push_back(add_vertex({xllcorner + (col * cellsize), yllcorner + (row * cellsize), height}));
		}
	}
	if (!f.good()) {
		throw std::runtime_error("Couldn't read terrain file");
	}
	for (size_t row = 1; row < nrows; ++row) {
		for (size_t col = 1; col < ncols; ++col) {
			add_face({
					vertices[ncols * (row - 1) + (col - 1)],
					vertices[ncols * (row - 0) + (col - 0)],
					vertices[ncols * (row - 0) + (col - 1)],
			});
			add_face({
					vertices[ncols * (row - 1) + (col - 1)],
					vertices[ncols * (row - 1) + (col - 0)],
					vertices[ncols * (row - 0) + (col - 0)],
			});
		}
	}
	update_face_normals();
	update_vertex_normals();
};

OpenMesh::FaceHandle
TerrainMesh::findPoint(glm::vec2 p) const
{
	return findPoint(p, *faces_begin());
}

TerrainMesh::PointFace::PointFace(const glm::vec2 p, const TerrainMesh * mesh) :
	PointFace {p, mesh, *mesh->faces_begin()}
{
}

TerrainMesh::PointFace::PointFace(const glm::vec2 p, const TerrainMesh * mesh, FaceHandle start) :
	PointFace {p, mesh->findPoint(p, start)}
{
}

TerrainMesh::FaceHandle
TerrainMesh::PointFace::face(const TerrainMesh * mesh, FaceHandle start) const
{
	if (_face.is_valid()) {
		assert(mesh->triangleContainsPoint(point, _face));
		return _face;
	}
	else {
		return (_face = mesh->findPoint(point, start));
	}
}

TerrainMesh::FaceHandle
TerrainMesh::PointFace::face(const TerrainMesh * mesh) const
{
	return face(mesh, *mesh->faces_begin());
}

namespace {
	[[nodiscard]] constexpr inline bool
	pointLeftOfLine(const glm::vec2 p, const glm::vec2 e1, const glm::vec2 e2)
	{
		return (e2.x - e1.x) * (p.y - e1.y) > (e2.y - e1.y) * (p.x - e1.x);
	}

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
TerrainMesh::findPoint(glm::vec2 p, OpenMesh::FaceHandle f) const
{
	ConstFaceVertexIter vertices;
	while (f.is_valid() && !triangleContainsPoint(p, vertices = cfv_iter(f))) {
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
TerrainMesh::positionAt(const PointFace & p) const
{
	glm::vec3 out {};
	Triangle<3> t {this, fv_range(p.face(this))};
	glm::intersectLineTriangle(p.point ^ 0.F, up, t[0], t[1], t[2], out);
	return p.point ^ out[0];
}

[[nodiscard]] std::optional<glm::vec3>
TerrainMesh::intersectRay(const Ray & ray) const
{
	return intersectRay(ray, findPoint(ray.start));
}

[[nodiscard]] std::optional<glm::vec3>
TerrainMesh::intersectRay(const Ray & ray, FaceHandle face) const
{
	std::optional<glm::vec3> out;
	walkUntil(PointFace {ray.start, face}, ray.start + (ray.direction * 10000.F), [&out, &ray, this](FaceHandle face) {
		glm::vec2 bari {};
		float dist {};
		Triangle<3> t {this, fv_range(face)};
		if (glm::intersectRayTriangle(ray.start, ray.direction, t[0], t[1], t[2], bari, dist)) {
			out = t * bari;
			return true;
		}
		return false;
	});
	return out;
}

void
TerrainMesh::walk(const PointFace & from, const glm::vec2 to, const std::function<void(FaceHandle)> & op) const
{
	walkUntil(from, to, [&op](const auto & fh) {
		op(fh);
		return false;
	});
}

void
TerrainMesh::walkUntil(const PointFace & from, const glm::vec2 to, const std::function<bool(FaceHandle)> & op) const
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
TerrainMesh::triangleContainsPoint(const glm::vec2 p, const glm::vec2 a, const glm::vec2 b, const glm::vec2 c)
{
	const auto det = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

	return det * ((b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x)) >= 0
			&& det * ((c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x)) >= 0
			&& det * ((a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x)) >= 0;
}

bool
TerrainMesh::triangleContainsPoint(const glm::vec2 p, FaceHandle face) const
{
	return triangleContainsPoint(p, cfv_iter(face));
}

bool
TerrainMesh::triangleContainsPoint(const glm::vec2 p, ConstFaceVertexIter vertices) const
{
	return triangleContainsPoint(p, point(*vertices++), point(*vertices++), point(*vertices++));
}
