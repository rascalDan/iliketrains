#include "factoryMesh.h"
#include "collections.hpp"
#include "gfx/models/vertex.hpp"
#include "modelFactoryMesh.h"

Mesh::Ptr
FactoryMesh::createMesh() const
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (const auto & use : uses) {
		ModelFactoryMesh mesh;
		use->createMesh(mesh, 1);

		if (!mesh.normalsProvidedProperty) {
			mesh.update_face_normals();
			mesh.update_vertex_normals();
		}
		for (const auto & face : mesh.faces()) {
			const auto & useVertexNormals
					= mesh.property(mesh.smoothFaceProperty, face) || mesh.normalsProvidedProperty;
			const auto & colour = mesh.color(face);
			const auto & material = mesh.property(mesh.materialFaceProperty, face);

			std::vector<unsigned int> faceIndices;
			for (const auto & heh : mesh.fh_range(face)) {
				const auto & vertex = mesh.to_vertex_handle(heh);
				const auto & textureUV = mesh.texcoord2D(heh);
				const auto & point = mesh.point(vertex);
				const auto & normal = useVertexNormals ? mesh.property(mesh.vertex_normals_pph(), vertex)
													   : mesh.property(mesh.face_normals_pph(), face);
				Vertex outVertex {point, textureUV, normal, colour, material};
				if (const auto existingItr = std::find(vertices.rbegin(), vertices.rend(), outVertex);
						existingItr != vertices.rend()) {
					faceIndices.push_back(static_cast<unsigned int>(std::distance(existingItr, vertices.rend()) - 1));
				}
				else {
					faceIndices.push_back(static_cast<unsigned int>(vertices.size()));
					vertices.emplace_back(outVertex);
				}
			}

			for (unsigned int i = 2; i < faceIndices.size(); i++) {
				indices.push_back(faceIndices[0]);
				indices.push_back(faceIndices[i - 1]);
				indices.push_back(faceIndices[i]);
			}
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
