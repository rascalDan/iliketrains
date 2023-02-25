#include "modelFactoryMesh.h"

ModelFactoryMesh::ModelFactoryMesh()
{
	add_property(smoothFaceProperty);
	add_property(nameFaceProperty);
}

std::pair<std::string, OpenMesh::FaceHandle>
ModelFactoryMesh::add_namedFace(std::string name, std::vector<OpenMesh::VertexHandle> p)
{
	const auto handle = add_face(std::move(p));
	property(nameFaceProperty, handle) = name;
	return std::make_pair(name, handle);
}
