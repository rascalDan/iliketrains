#include "cuboid.h"
#include "assetFactoryConfig.h"

Cuboid::CreatedFaces
Cuboid::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	static constexpr std::array<glm::vec3, 8> VERTICES {{
			// bottom
			{n, n, z},
			{n, y, z},
			{y, y, z},
			{y, n, z},
			// top
			{y, n, o},
			{y, y, o},
			{n, y, o},
			{n, n, o},
	}};

	const auto vhs = addMutatedToMesh(mesh, VERTICES, mutation);
	return {
			{"top", mesh.add_face({vhs[4], vhs[5], vhs[6], vhs[7]})},
			{"bottom", mesh.add_face({vhs[0], vhs[1], vhs[2], vhs[3]})},
			{"left", mesh.add_face({vhs[0], vhs[7], vhs[6], vhs[1]})},
			{"right", mesh.add_face({vhs[2], vhs[5], vhs[4], vhs[3]})},
			{"front", mesh.add_face({vhs[0], vhs[3], vhs[4], vhs[7]})},
			{"back", mesh.add_face({vhs[2], vhs[1], vhs[6], vhs[5]})},
	};
}
