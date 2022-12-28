#include "sceneProvider.h"
#include "sceneRenderer.h"

void
SceneProvider::environment(const SceneShader &, const SceneRenderer & renderer) const
{
	renderer.setAmbientLight({0.5F, 0.5F, 0.5F});
	renderer.setDirectionalLight({0.6F, 0.6F, 0.6F}, {-1, 1, -1}, *this);
}
void
SceneProvider::shadows(const ShadowMapper &) const
{
}
