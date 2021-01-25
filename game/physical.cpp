#include "physical.h"
#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include <cache.h>
#include <gfx/gl/shader.h>
#include <gfx/gl/transform.h>

Cache<Mesh> Physical::cachedMesh;

Physical::Physical(glm::vec3 where, const std::string & m, const std::string & t) :
	location {where}, mesh {cachedMesh.get(m)}, texture {Texture::cachedTexture.get(t)}
{
}

void
Physical::render(const Shader & shader) const
{
	shader.setModel(location.GetModel());
	texture->Bind();
	mesh->Draw();
}
