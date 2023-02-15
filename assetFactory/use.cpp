#include "use.h"

Shape::CreatedFaces
Use::createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const
{
	auto faces = type->createMesh(mesh, mutation * getMatrix());
	for (const auto & [name, faceController] : faceControllers) {
		faceController.apply(mesh, name, faces);
	}
	return faces;
}
