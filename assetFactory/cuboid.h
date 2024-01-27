#pragma once

#include "shape.h"

class Cuboid : public Shape {
public:
	CreatedFaces createMesh(ModelFactoryMesh & mesh, Scale3D lodf) const override;
};
