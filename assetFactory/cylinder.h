#pragma once

#include "shape.h"

class Cylinder : public Shape {
public:
	CreatedFaces createMesh(ModelFactoryMesh & mesh, float lodf) const override;
};
