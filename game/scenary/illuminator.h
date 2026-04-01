#pragma once

#include "assetFactory/asset.h"
#include "gfx/gl/instanceVertices.h"
#include "gfx/models/lights.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"

class SceneShader;
class Location;

class Illuminator : public Asset, public Renderable, public StdTypeDefs<Illuminator> {
	Mesh::Ptr bodyMesh;
	Texture::Ptr texture;
	std::shared_ptr<glVertexArray> instanceVAO;
	static std::weak_ptr<glVertexArray> commonInstanceVAO;

public:
	[[nodiscard]] std::any createAt(const Location &) const override;

	struct SpotLight : Persistence::Persistable, SpotLightDef, StdTypeDefs<SpotLight> {
	private:
		friend Persistence::SelectionPtrBase<std::shared_ptr<SpotLight>>;
		bool persist(Persistence::PersistenceStore & store) override;
	};

	struct PointLight : Persistence::Persistable, PointLightDef, StdTypeDefs<PointLight> {
	private:
		friend Persistence::SelectionPtrBase<std::shared_ptr<PointLight>>;
		bool persist(Persistence::PersistenceStore & store) override;
	};

	struct InstanceVertex {
		CommonLocationInstance location;
	};

	mutable InstanceVertices<InstanceVertex> instances;
	void render(const SceneShader &, const Frustum &) const override;

protected:
	friend Persistence::SelectionPtrBase<std::shared_ptr<Illuminator>>;
	bool persist(Persistence::PersistenceStore & store) override;
	void postLoad() override;

public:
	std::vector<SpotLight::Ptr> spotLight;
	std::vector<PointLight::Ptr> pointLight;
};
