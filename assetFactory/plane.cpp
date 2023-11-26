#include "plane.h"
#include "modelFactoryMesh.h"

Plane::CreatedFaces
Plane::createMesh(ModelFactoryMesh & mesh, float) const
{
	static constexpr std::array<Position3D, 4> VERTICES {{
			{n, n, z},
			{y, n, z},
			{y, y, z},
			{n, y, z},
	}};

	return {mesh.add_namedFace("plane", addToMesh(mesh, VERTICES))};
}
