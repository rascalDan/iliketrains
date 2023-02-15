#pragma once

#include "shape.h"

class Plane : public Shape {
public:
	CreatedFaces createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const override;
};
