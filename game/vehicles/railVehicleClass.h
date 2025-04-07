#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"
#include <array>
#include <memory>

class SceneShader;
class ShadowMapper;
class Location;

class RailVehicleClass : public Renderable, public Asset {
public:
	void render(const SceneShader & shader, const Frustum &) const override;
	void shadows(const ShadowMapper & shadowMapper, const Frustum &) const override;

	[[nodiscard]] std::any createAt(const Location &) const override;

	struct LocationVertex {
		glm::mat3 body, front, back;
		GlobalPosition3D bodyPos, frontPos, backPos;
	};

	std::array<Mesh::Ptr, 2> bogies;
	Mesh::Ptr bodyMesh;
	Texture::Ptr texture;
	float wheelBase;
	float length;
	float maxSpeed;

	mutable InstanceVertices<LocationVertex> instances;
	using Instance = decltype(instances)::InstanceProxy;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<RailVehicleClass>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;

private:
	glVertexArray instanceVAO;
	std::array<glVertexArray, 2> instancesBogiesVAO;
};

using RailVehicleClassPtr = std::shared_ptr<RailVehicleClass>;
