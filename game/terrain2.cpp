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
	auto vertices = cfv_iter(face);
	return triangleContainsPoint(p, point(*vertices++), point(*vertices++), point(*vertices++));
}
