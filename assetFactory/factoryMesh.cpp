#include "factoryMesh.h"
#include "collections.hpp"
#include "gfx/models/vertex.hpp"
#include "modelFactoryMesh.h"
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
	mesh.update_vertex_normals();
	std::vector<Vertex> vertices;
	for (const auto & face : mesh.faces()) {
		const auto smooth = mesh.property(mesh.smoothFaceProperty, face);
		const auto colour = mesh.color(face);
		for (const auto & vertex : mesh.fv_range(face)) {
			vertices.emplace_back(mesh.point(vertex), NullUV,
					smooth ? mesh.property(mesh.vertex_normals_pph(), vertex)
						   : mesh.property(mesh.face_normals_pph(), face),
					colour);
		}
	}
	return std::make_shared<Mesh>(vertices, vectorOfN(vertices.size()));
}

bool
FactoryMesh::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_MEMBER(size)
			&& STORE_NAME_HELPER("use", uses, Persistence::Appender<Use::Collection>);
}
