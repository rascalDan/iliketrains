#ifndef PHYSICAL_H
#define PHYSICAL_H

#include "gfx/models/mesh.h"
#include "gfx/models/texture.h"
#include <gfx/gl/transform.h>
#include <glm/glm.hpp>
#include <string>

class Camera;
class Shader;

class Physical {
public:
	Physical(glm::vec3 where, const std::string & m, const std::string & t);

	void render(const Shader & shader, const Camera & camera) const;

	[[nodiscard]] const auto &
	getPosition() const
	{
		return location.GetPos();
	}

protected:
	Transform location;
	Mesh mesh;
	Texture texture;
};

#endif
