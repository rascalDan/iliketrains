#include "shape.h"
#include "gfx/models/vertex.hpp"
#include "maths.h"
#include "modelFactoryMesh.h"
#include "shape.h"

std::vector<OpenMesh::VertexHandle>
Shape::addToMesh(ModelFactoryMesh & mesh, const std::span<const glm::vec3> vertices)
{
	std::vector<OpenMesh::VertexHandle> vhs;
	std::transform(vertices.begin(), vertices.end(), std::back_inserter(vhs), [&mesh](const auto & p) {
		return mesh.add_vertex({p.x, p.y, p.z});
	});
	return vhs;
}
