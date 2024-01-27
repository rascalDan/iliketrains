#pragma once

#include "shape.h"

class Plane : public Shape {
public:
	CreatedFaces createMesh(ModelFactoryMesh & mesh, Scale3D lodf) const override;
};
