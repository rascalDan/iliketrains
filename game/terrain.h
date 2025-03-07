#pragma once

#include "chronology.h"
#include "config/types.h"
#include "game/worldobject.h"
#include "geoData.h"
#include "gfx/models/texture.h"
#include "gfx/renderable.h"

class SceneShader;

class Terrain : public GeoData, public WorldObject, public Renderable {
public:
	template<typename... P> explicit Terrain(P &&... params) : GeoData {std::forward<P>(params)...}
	{
		generateMeshes();
	}

	void render(const SceneShader & shader, const Frustum &) const override;
	void shadows(const ShadowMapper &) const override;

	void tick(TickDuration) override;

	struct Vertex {
		GlobalPosition3D pos;
		Normal3D normal;
	};

	void generateMeshes();

private:
	void afterChange() override;

	struct SurfaceArrayBuffer {
		glVertexArray vertexArray;
		glBuffer indicesBuffer;
		GLsizei count;
		AxisAlignedBoundingBox aabb;
		mutable bool visible;
	};

	struct SurfaceKey {
		const Surface * surface;
		GlobalPosition2D basePosition;
		bool operator<(const SurfaceKey &) const;
	};

	glBuffer verticesBuffer;
	std::map<SurfaceKey, SurfaceArrayBuffer> meshes;
	Texture::Ptr grass = std::make_shared<Texture>("grass.png");
};
