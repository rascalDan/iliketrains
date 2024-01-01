#pragma once

#include "config/types.h"
#include "persistence.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Mutation {
	using Matrix = glm::mat4;

	[[nodiscard]] Matrix getMatrix() const;
	[[nodiscard]] Matrix getDeformationMatrix() const;
	[[nodiscard]] Matrix getLocationMatrix() const;
	[[nodiscard]] float relativeLevelOfDetail() const;

	Position3D position {};
	Rotation3D rotation {};
	Scale3D scale {1};

protected:
	bool persist(Persistence::PersistenceStore & store);
};
