#include "plant.h"

void
Plant::render(const SceneShader & shader) const
{
	type->render(shader, position);
}

void
Plant::shadows(const ShadowMapper & mapper) const
{
	type->shadows(mapper, position);
}
