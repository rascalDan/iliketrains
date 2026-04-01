#include "renderable.h"
#include "gl_traits.h"
#include "location.h"
#include "maths.h"
#include "util.h"

std::weak_ptr<Renderable::CommonLocationData> Renderable::commonLocationData;

Renderable::CommonLocation::CommonLocation(Location const & location) :
	position {location.pos, 0}, rotation {location.rot, 0}, rotationMatrix {location.getRotationTransform()}
{
}

Renderable::CommonLocation &
Renderable::CommonLocation ::operator=(Location const & location)
{
	position = location.pos || 0;
	rotation = location.rot || 0.F;
	rotationMatrix = location.getRotationTransform();
	return *this;
}

Renderable::Renderable()
{
	createIfRequired(locationData, commonLocationData);
}

GLuint
gl_traits<InstanceVertices<Renderable::CommonLocation>::InstanceProxy>::vertexArrayAttribFormat(
		GLuint vao, GLuint index, GLuint offset)
{
	return gl_traits<
			decltype(InstanceVertices<Renderable::CommonLocation>::InstanceProxy::index)>::vertexArrayAttribFormat(vao,
			index, offset + offsetof(InstanceVertices<Renderable::CommonLocation>::InstanceProxy, index));
};

void
Renderable::preFrame(const Frustum &, const Frustum &)
{
}

void
Renderable::lights(const SceneShader &) const
{
}

void
Renderable::shadows(const ShadowMapper &, const Frustum &) const
{
}

void
Renderable::updateStencil(const ShadowStenciller &) const
{
}

void
Renderable::updateBillboard(const BillboardPainter &) const
{
}
