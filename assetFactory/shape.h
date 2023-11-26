#pragma once

#include "config/types.h"
#include "modelFactoryMesh_fwd.h"
#include "stdTypeDefs.h"
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <glm/vec3.hpp>
#include <map>
#include <span>
#include <string>

class Vertex;

class Shape : public StdTypeDefs<Shape> {
public:
	using CreatedFaces = std::multimap<std::string, OpenMesh::FaceHandle>;

	static constexpr float z {}, y {.5}, n {-y}, o {1};

	virtual ~Shape() = default;

	virtual CreatedFaces createMesh(ModelFactoryMesh &, float levelOfDetailFactor) const = 0;

	static std::vector<OpenMesh::VertexHandle> addToMesh(
			ModelFactoryMesh & mesh, const std::span<const Position3D> vertices);
};
