#include "factoryMesh.h"
#include "collections.hpp"
#include "gfx/models/vertex.hpp"
#include "modelFactoryMesh.h"

Mesh::Ptr
FactoryMesh::createMesh() const
{
	ModelFactoryMesh mesh;
	for (const auto & use : uses) {
		use->createMesh(mesh, 1);
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
			const auto textureUV = mesh.texcoord2D(vertex);
			vertices.emplace_back(mesh.point(vertex), textureUV,
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
