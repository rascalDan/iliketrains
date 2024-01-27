#pragma once

#include "shape.h"

class Cylinder : public Shape {
public:
	CreatedFaces createMesh(ModelFactoryMesh & mesh, Scale3D lodf) const override;
};
