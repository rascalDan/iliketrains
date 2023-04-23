#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/models/mesh.h"
#include "gfx/renderable.h"
#include <array>
#include <memory>
#include <string>

class SceneShader;
class ShadowMapper;
class Texture;
class Location;

class RailVehicleClass : public Renderable, public Asset {
public:
	void render(const SceneShader & shader) const override;
	void shadows(const ShadowMapper & shadowMapper) const override;

	std::array<Mesh::Ptr, 2> bogies;
	Mesh::Ptr bodyMesh;
	std::shared_ptr<Texture> texture;
	float wheelBase;
	float length;
	float maxSpeed;

	mutable InstanceVertices<glm::mat4> instancesBody;
	mutable std::array<InstanceVertices<glm::mat4>, 2> instancesBogies;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<RailVehicleClass>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;

private:
	glVertexArray instanceVAO;
	std::array<glVertexArray, 2> instancesBogiesVAO;
};
using RailVehicleClassPtr = std::shared_ptr<RailVehicleClass>;
