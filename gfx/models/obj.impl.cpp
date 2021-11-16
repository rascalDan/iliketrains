#include "obj.h"
#include <algorithm>
#include <gfx/models/mesh.h> // IWYU pragma: keep
#include <gfx/models/vertex.hpp>
#include <glm/glm.hpp>
#include <iterator>
#include <map>
#include <memory>
#include <utility>
#include <vector>

ObjParser::ObjParser(const std::filesystem::path & fileName) : ObjParser {std::make_unique<std::ifstream>(fileName)} { }

ObjParser::ObjParser(std::unique_ptr<std::istream> in) : yyFlexLexer(in.get())
{
	assert(in);
	ObjParser::yylex();
	assert(in->good());
	std::for_each(vertices.begin(), vertices.end(), [](auto & v) {
		std::swap(v.y, v.z);
		v.x = -v.x;
	});
}

ObjParser::NamedMeshes
ObjParser::createMeshes() const
{
	NamedMeshes out;
	const auto data {createMeshData()};
	std::transform(data.begin(), data.end(), std::inserter(out, out.end()), [](auto && obj) {
		return std::make_pair(obj.first, std::make_shared<Mesh>(obj.second.first, obj.second.second));
	});
	return out;
}

ObjParser::NamedMeshesData
ObjParser::createMeshData() const
{
	NamedMeshesData out;
	std::transform(objects.begin(), objects.end(), std::inserter(out, out.end()), [this](auto && obj) {
		std::vector<Vertex> overtices;
		std::vector<ObjParser::FaceElement> vertexOrder;
		std::vector<unsigned int> indices;
		for (const auto & face : obj.second) {
			for (auto idx = 2U; idx < face.size(); idx += 1) {
				auto f = [&](auto idx) {
					const auto & fe {face[idx]};
					if (const auto existing = std::find(vertexOrder.begin(), vertexOrder.end(), fe);
							existing != vertexOrder.end()) {
						indices.push_back(std::distance(vertexOrder.begin(), existing));
					}
					else {
						indices.push_back(overtices.size());
						overtices.emplace_back(vertices[fe.x - 1], texCoords[fe.y - 1], -normals[fe.z - 1]);
						vertexOrder.emplace_back(fe);
					}
				};
				f(0);
				f(idx);
				f(idx - 1);
			}
		}
		return std::make_pair(obj.first, std::make_pair(overtices, indices));
	});
	return out;
}
