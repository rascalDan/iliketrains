#include "plane.h"
#include "modelFactoryMesh.h"

Plane::CreatedFaces
Plane::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	static constexpr std::array<glm::vec3, 4> VERTICES {{
			{n, n, z},
			{y, n, z},
			{y, y, z},
			{n, y, z},
	}};

	return {mesh.add_namedFace("plane", addMutatedToMesh(mesh, VERTICES, mutation))};
}