#pragma once

#include "gfx/gl/instanceVertices.h"
#include "gl_traits.h"
#include <glm/mat3x3.hpp>
#include <special_members.h>

class SceneShader;
class Frustum;
class ShadowMapper;
class ShadowStenciller;
class BillboardPainter;
class Location;

class Renderable {
public:
	Renderable();
	virtual ~Renderable() = default;
	DEFAULT_MOVE_COPY(Renderable);

	virtual void preFrame(const Frustum &, const Frustum &);
	virtual void render(const SceneShader & shader, const Frustum &) const = 0;
	virtual void lights(const SceneShader & shader) const;
	virtual void shadows(const ShadowMapper & shadowMapper, const Frustum &) const;

	virtual void updateStencil(const ShadowStenciller & lightDir) const;
	virtual void updateBillboard(const BillboardPainter &) const;

	struct CommonLocation {
		CommonLocation(const Location &);
		CommonLocation & operator=(const Location &);

		glm::ivec4 position;
		glm::vec4 rotation;
		glm::mat3x4 rotationMatrix;
	};

	using CommonLocationData = InstanceVertices<CommonLocation>;
	using CommonLocationInstance = CommonLocationData::InstanceProxy;

	std::shared_ptr<CommonLocationData> locationData;

	static std::weak_ptr<CommonLocationData> commonLocationData;
};

template<> struct gl_traits<InstanceVertices<Renderable::CommonLocation>::InstanceProxy> {
	static GLuint vertexArrayAttribFormat(GLuint vao, GLuint index, GLuint offset);
};

