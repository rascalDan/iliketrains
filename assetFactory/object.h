#pragma once

#include "shape.h"
#include "stdTypeDefs.hpp"
#include "use.h"

class Object : public StdTypeDefs<Object>, public Shape {
public:
	Object(std::string i);

	CreatedFaces createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const override;

	Use::Collection uses;
	std::string id;
};
