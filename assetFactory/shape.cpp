#include "shape.h"
#include "gfx/models/vertex.hpp"
#include "maths.h"
#include "modelFactoryMesh.h"
#include "shape.h"

std::vector<OpenMesh::VertexHandle>
Shape::addMutatedToMesh(
		ModelFactoryMesh & mesh, const std::span<const glm::vec3> vertices, const Mutation::Matrix & mutation)
{
	std::vector<OpenMesh::VertexHandle> vhs;
	std::transform(vertices.begin(), vertices.end(), std::back_inserter(vhs), [&mesh, &mutation](const auto & v) {
		const auto p = v % mutation;
		return mesh.add_vertex({p.x, p.y, p.z});
	});
	return vhs;
}
