#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <gfx/gl/transform.h>
#include <gfx/renderable.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Shader;
class Mesh;
class Texture;
template<typename Obj> class Cache;

class Physical : public Renderable {
public:
	Physical(glm::vec3 where, const std::string & m, const std::string & t);

	void render(const Shader & shader) const override;

	[[nodiscard]] const auto &
	getPosition() const
	{
		return location.GetPos();
	}

protected:
	Transform location;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Texture> texture;

private:
	static Cache<Mesh> cachedMesh;
};

#endif
