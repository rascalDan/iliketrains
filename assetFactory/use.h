#pragma once

#include "assetFactoryConfig_fwd.h"
#include "faceController.h"
#include "shape.h"
#include "stdTypeDefs.hpp"

class Use : public StdTypeDefs<Use>, public Mutation {
public:
	using FaceControllers = std::map<std::string, FaceController>;

	Shape::CreatedFaces createMesh(ModelFactoryMesh & mesh, const Mutation::Matrix & mutation) const;

	Shape::CPtr type;
	std::string colour;
	FaceControllers faceControllers;
};
