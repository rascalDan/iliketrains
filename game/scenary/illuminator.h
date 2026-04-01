#pragma once

#include "assetFactory/asset.h"
#include "assetFactory/lights.h"
#include "gfx/gl/instanceVertices.h"
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
