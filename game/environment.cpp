#include "environment.h"
#include <chronology.h>
#include <gfx/gl/sceneRenderer.h>

Environment::Environment() : worldTime {"2024-01-01T12:00:00"_time_t} { }

void
Environment::tick(TickDuration)
{
	worldTime += 1;
}

void
Environment::render(const SceneRenderer & renderer, const SceneProvider & scene) const
{
	renderer.setAmbientLight({0.5F, 0.5F, 0.5F});
	renderer.setDirectionalLight({0.6F, 0.6F, 0.6F}, {-1, 1, -1}, scene);
}
