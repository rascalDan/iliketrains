#include "factoryMesh.h"
#include "assetFactoryConfig.h"
#include "collections.hpp"
#include "gfx/models/vertex.hpp"
#include <glm/ext/matrix_transform.hpp>

Mesh::Ptr
FactoryMesh::createMesh() const
{
	constexpr glm::vec2 NullUV {};

	ModelFactoryMesh mesh;
	for (const auto & use : uses) {
		use->createMesh(mesh, glm::identity<Mutation::Matrix>());
	}
	mesh.garbage_collection();

	mesh.triangulate();
	mesh.update_face_normals();
	std::vector<Vertex> vertices;
	for (const auto & face : mesh.faces()) {
		for (const auto & vertex : mesh.fv_range(face)) {
			vertices.emplace_back(mesh.point(vertex), NullUV, mesh.property(mesh.face_normals_pph(), face));
		}
	}
	return std::make_shared<Mesh>(vertices, vectorOfN(vertices.size()));
}
