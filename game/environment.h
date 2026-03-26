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
	[[nodiscard]] Direction2D getSunPos() const;
	[[nodiscard]] time_t getWorldTime() const;
	[[nodiscard]] static Direction2D getSunPos(Direction2D position, time_t time);

private:
	time_t worldTime;
	glm::vec<2, Angle> earthPos;
};
