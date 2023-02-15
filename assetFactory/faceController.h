#pragma once

#include "assetFactoryConfig_fwd.h"
#include "mutation.h"
#include "shape.h"
#include <map>
#include <string>

class FaceController : public Mutation {
public:
	using FaceControllers = std::map<std::string, FaceController>;

	void apply(ModelFactoryMesh & mesh, const std::string & name, Shape::CreatedFaces & faces) const;

	std::string id;
	std::string colour;
	std::string type;
	bool smooth {false};
	FaceControllers faceControllers;
};
