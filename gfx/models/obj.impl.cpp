#include "obj.h"
#include <algorithm>
#include <gfx/models/mesh.h>
#include <gfx/models/vertex.hpp>
#include <glm/glm.hpp>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

std::vector<ObjParser::NamedMesh>
ObjParser::createMeshes() const
{
	std::vector<ObjParser::NamedMesh> out;
	out.reserve(objects.size());
	for (const auto & obj : objects) {
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
		out.emplace_back(obj.first, std::make_shared<Mesh>(overtices, indices));
	}
	return out;
}
