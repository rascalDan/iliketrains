#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/renderable.h"

class SceneShader;
class Location;
class Texture;

class Illuminator : public Asset, public Renderable, public StdTypeDefs<Illuminator> {
	Mesh::Ptr bodyMesh;
	std::shared_ptr<Texture> texture;
	glVertexArray instanceVAO, instancesSpotLightVAO;

public:
	struct SpotLightVertex {
		RelativePosition3D position;
		Direction3D direction;
		RGB colour;
		RelativeDistance kq;
		Angle arc;
	};

	struct SpotLight : Persistence::Persistable, SpotLightVertex, StdTypeDefs<SpotLight> {
	private:
		friend Persistence::SelectionPtrBase<std::shared_ptr<SpotLight>>;
		bool persist(Persistence::PersistenceStore & store) override;
	};

public:
	using LocationVertex = std::pair<glm::mat4, GlobalPosition3D>;
	mutable InstanceVertices<LocationVertex> instances;
	mutable InstanceVertices<SpotLightVertex> instancesSpotLight;
	void render(const SceneShader &) const override;
	void lights(const SceneShader &) const override;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Illuminator>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;

	std::vector<SpotLight::Ptr> spotLight;
	std::vector<InstanceVertices<SpotLightVertex>::InstanceProxy> spotLightInstances;
};
