#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"

class SceneShader;
class Location;

class Illuminator : public Asset, public Renderable, public StdTypeDefs<Illuminator> {
	Mesh::Ptr bodyMesh;
	Texture::Ptr texture;
	glVertexArray instanceVAO;
	std::optional<glVertexArray> instancesSpotLightVAO, instancesPointLightVAO;

public:
	struct LightCommonVertex {
		RelativePosition3D position;
		RGB colour;
		RelativeDistance kq;
	};

	struct SpotLightVertex : LightCommonVertex {
		Direction3D direction;
		Angle arc;
	};

	struct PointLightVertex : LightCommonVertex { };

	struct SpotLight : Persistence::Persistable, SpotLightVertex, StdTypeDefs<SpotLight> {
	private:
		friend Persistence::SelectionPtrBase<std::shared_ptr<SpotLight>>;
		bool persist(Persistence::PersistenceStore & store) override;
	};

	struct PointLight : Persistence::Persistable, PointLightVertex, StdTypeDefs<PointLight> {
	private:
		friend Persistence::SelectionPtrBase<std::shared_ptr<PointLight>>;
		bool persist(Persistence::PersistenceStore & store) override;
	};

public:
	using LocationVertex = std::pair<glm::mat3, GlobalPosition3D>;
	mutable InstanceVertices<LocationVertex> instances;
	mutable InstanceVertices<SpotLightVertex> instancesSpotLight;
	mutable InstanceVertices<PointLightVertex> instancesPointLight;
	void render(const SceneShader &) const override;
	void lights(const SceneShader &) const override;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Illuminator>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;

	std::vector<SpotLight::Ptr> spotLight;
	std::vector<PointLight::Ptr> pointLight;
	std::vector<InstanceVertices<SpotLightVertex>::InstanceProxy> spotLightInstances;
	std::vector<InstanceVertices<PointLightVertex>::InstanceProxy> pointLightInstances;
};
