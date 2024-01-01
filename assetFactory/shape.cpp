#include "shape.h"
#include "modelFactoryMesh.h"
#include "shape.h"

std::vector<OpenMesh::VertexHandle>
Shape::addToMesh(ModelFactoryMesh & mesh, const std::span<const RelativePosition3D> vertices)
{
	std::vector<OpenMesh::VertexHandle> vhs;
	std::transform(vertices.begin(), vertices.end(), std::back_inserter(vhs), [&mesh](const auto & p) {
		return mesh.add_vertex(p);
	});
	return vhs;
}
