#include "cuboid.h"
#include "modelFactoryMesh.h"

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
			mesh.add_namedFace("top", {vhs[4], vhs[5], vhs[6], vhs[7]}),
			mesh.add_namedFace("bottom", {vhs[0], vhs[1], vhs[2], vhs[3]}),
			mesh.add_namedFace("left", {vhs[0], vhs[7], vhs[6], vhs[1]}),
			mesh.add_namedFace("right", {vhs[2], vhs[5], vhs[4], vhs[3]}),
			mesh.add_namedFace("front", {vhs[0], vhs[3], vhs[4], vhs[7]}),
			mesh.add_namedFace("back", {vhs[2], vhs[1], vhs[6], vhs[5]}),
	};
}
