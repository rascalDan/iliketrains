#pragma once

#include "assetFactoryConfig_fwd.h"
#include "mutation.h"
#include "stdTypeDefs.hpp"
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <map>
#include <span>
#include <string>

class Vertex;

class Shape : public StdTypeDefs<Shape> {
public:
	using CreatedFaces = std::multimap<std::string, OpenMesh::FaceHandle>;

	static constexpr float z {}, y {.5}, n {-y}, o {1};

	virtual ~Shape() = default;

	virtual CreatedFaces createMesh(ModelFactoryMesh &, const Mutation::Matrix & mutation) const = 0;

	static std::vector<OpenMesh::VertexHandle> addMutatedToMesh(
			ModelFactoryMesh & mesh, const std::span<const glm::vec3> vertices, const Mutation::Matrix & mutation);
};
