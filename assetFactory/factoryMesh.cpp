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

	mesh.update_face_normals();
	mesh.update_vertex_normals();
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	for (const auto & face : mesh.faces()) {
		const auto smooth = mesh.property(mesh.smoothFaceProperty, face);
		const auto colour = mesh.color(face);
		auto hrange = mesh.fh_range(face);
		const unsigned int start = static_cast<unsigned int>(vertices.size());
		for (const auto & heh : hrange) {
			const auto & vertex = mesh.to_vertex_handle(heh);
			const auto textureUV = mesh.texcoord2D(heh);
			vertices.emplace_back(mesh.point(vertex), textureUV,
					smooth ? mesh.property(mesh.vertex_normals_pph(), vertex)
						   : mesh.property(mesh.face_normals_pph(), face),
					colour);
		}
		const auto vcount = std::distance(hrange.begin(), hrange.end());
		for (unsigned int i = 2; i < vcount; i++) {
			indices.push_back(start);
			indices.push_back(start + i - 1);
			indices.push_back(start + i);
		}
	}
	return std::make_shared<Mesh>(vertices, indices);
}

bool
FactoryMesh::persist(Persistence::PersistenceStore & store)
{
	return STORE_TYPE && STORE_MEMBER(id) && STORE_MEMBER(size)
			&& STORE_NAME_HELPER("use", uses, Persistence::Appender<Use::Collection>);
}
