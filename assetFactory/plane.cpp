#include "plane.h"
#include "assetFactoryConfig.h"

Plane::CreatedFaces
Plane::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	static constexpr std::array<glm::vec3, 4> VERTICES {{
			{n, n, z},
			{y, n, z},
			{y, y, z},
			{n, y, z},
	}};

	return {{"plane", mesh.add_face(addMutatedToMesh(mesh, VERTICES, mutation))}};
}
