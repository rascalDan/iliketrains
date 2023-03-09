#pragma once

#include "shape.h"

class Plane : public Shape {
public:
	CreatedFaces createMesh(ModelFactoryMesh & mesh, float lodf) const override;
};
