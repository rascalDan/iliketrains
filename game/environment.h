#pragma once

#include "worldobject.h"

class SceneRenderer;
class SceneProvider;

class Environment : public WorldObject {
public:
	Environment();
	void tick(TickDuration elapsed) override;
	void render(const SceneRenderer &, const SceneProvider &) const;

private:
	time_t worldTime;
};
