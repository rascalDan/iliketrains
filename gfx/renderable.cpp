#include "renderable.h"

void
Renderable::preFrame(const Frustum &)
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
