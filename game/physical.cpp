#include "physical.h"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include <cache.h>
#include <gfx/gl/shader.h>

Cache<Mesh> Physical::cachedMesh;
Cache<Texture> Physical::cachedTexture;

Physical::Physical(glm::vec3 where, const std::string & m, const std::string & t) :
	location {where}, mesh {cachedMesh.get(m)}, texture {cachedTexture.get(t)}
{
}

void
Physical::render(const Shader & shader, const Camera & camera) const
{
	shader.Update(location, camera);
	texture->Bind();
	mesh->Draw();
}
