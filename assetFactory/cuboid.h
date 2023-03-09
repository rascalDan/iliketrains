#pragma once

#include "shape.h"

class Cuboid : public Shape {
public:
	CreatedFaces createMesh(ModelFactoryMesh & mesh, float lodf) const override;
};
