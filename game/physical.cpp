#include "physical.h"
#include <gfx/gl/shader.h>

Physical::Physical(glm::vec3 where, const std::string & m, const std::string & t) :
	location {where}, mesh {m}, texture {t}
{
}

void
Physical::render(const Shader & shader, const Camera & camera) const
{
	shader.Update(location, camera);
	texture.Bind();
	mesh.Draw();
}
