#include "terrain2.h"
#include <fstream>

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
				if ((e2.x - e1.x) * (p.y - e1.y) > (e2.y - e1.y) * (p.x - e1.x)) {
					break;
				}
			}
			f.reset();
		}
	}
	return f;
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
