#pragma once

#include "config/types.h"
#include "worldobject.h"

class SceneRenderer;
class SceneProvider;

class Environment : public WorldObject {
public:
	Environment();
	void tick(TickDuration elapsed) override;
	void render(const SceneRenderer &, const SceneProvider &) const;
	static Direction2D getSunPos(const Direction2D position, const time_t time);

private:
	time_t worldTime;
};
