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
	void shadows(const ShadowMapper &, const Frustum &) const override;

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
		AxisAlignedBoundingBox<GlobalDistance> aabb;
	};

	struct SurfaceKey {
		const Surface * surface;
		GlobalPosition2D basePosition;
		inline bool operator<(const SurfaceKey &) const;
	};

	using SurfaceIndices = std::map<SurfaceKey, std::vector<GLuint>>;
	void copyVerticesToBuffer() const;
	[[nodiscard]] SurfaceIndices mapSurfaceFacesToIndices() const;
	void copyIndicesToBuffers(const SurfaceIndices &);
	void pruneOrphanMeshes(const SurfaceIndices &);
	[[nodiscard]] inline GlobalPosition2D getTile(const FaceHandle &) const;

	glBuffer verticesBuffer;
	std::map<SurfaceKey, SurfaceArrayBuffer> meshes;
	Texture::Ptr grass = std::make_shared<Texture>("grass.png");
};
